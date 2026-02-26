#include <drogon/drogon.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <csignal>

#include "config_manager.h"
#include "db_pool.h"
#include "cors_filter.h"
#include "controllers/ui_api_controller.h"
#include "controllers/media_controller.h"

namespace fs = std::filesystem;

namespace {

void setup_logging(const yolo::LoggingConfig& log_config) {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    if (!log_config.file.empty()) {
        auto dir = fs::path(log_config.file).parent_path();
        if (!dir.empty()) fs::create_directories(dir);
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_config.file, log_config.max_bytes, log_config.backup_count));
    }

    auto logger = std::make_shared<spdlog::logger>("yolo-timeline", sinks.begin(), sinks.end());

    spdlog::level::level_enum level = spdlog::level::info;
    if (log_config.level == "DEBUG" || log_config.level == "debug") level = spdlog::level::debug;
    else if (log_config.level == "WARNING" || log_config.level == "warning") level = spdlog::level::warn;
    else if (log_config.level == "ERROR" || log_config.level == "error") level = spdlog::level::err;

    logger->set_level(level);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(3));
}

std::string find_config_path(int argc, char* argv[]) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--config") return argv[i + 1];
    }
    if (fs::exists("config.yaml")) return "config.yaml";
    if (fs::exists("/app/config/config.yaml")) return "/app/config/config.yaml";
    if (fs::exists("/opt/yolo_detection/config.yaml")) return "/opt/yolo_detection/config.yaml";
    return "config.yaml";
}

// Read index.html and optionally replace <base href="/"> with the HA ingress path.
// Returns empty string if index.html doesn't exist.
std::string load_index_html(const std::string& static_abs, const std::string& ingress_path) {
    auto index_path = fs::path(static_abs) / "index.html";
    std::ifstream f(index_path);
    if (!f.is_open()) return {};

    std::string html((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    if (!ingress_path.empty()) {
        // Ensure trailing slash — required for <base href> to work as a directory
        std::string base_href = ingress_path;
        if (base_href.back() != '/') base_href += '/';

        // Replace Angular's compiled <base href="/">
        const std::string needle = R"(<base href="/">)";
        auto pos = html.find(needle);
        if (pos != std::string::npos) {
            html.replace(pos, needle.size(), R"(<base href=")" + base_href + R"(">)");
        }
    }
    return html;
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    try {
        auto config_path = find_config_path(argc, argv);
        auto config = yolo::ConfigManager::load(config_path);

        setup_logging(config.logging);
        spdlog::info("Starting yolo-timeline service v1.0.0");
        spdlog::info("Config: {}", config_path);

        // Database pool
        yolo::DbPool::Config db_config{
            .host = config.database.host,
            .port = config.database.port,
            .user = config.database.user,
            .password = config.database.password,
            .database = config.database.database,
            .pool_size = config.database.pool_size,
        };
        auto db_pool = std::make_shared<yolo::DbPool>(db_config);

        // Configure controllers with shared dependencies
        yolo::UiApiController::setDbPool(db_pool);
        yolo::UiApiController::setDetectionServiceUrl(config.timeline.detection_service_url);
        yolo::MediaController::setEventsDir(config.timeline.events_dir);
        yolo::MediaController::setSnapshotsDir(config.timeline.snapshots_dir);
        yolo::CorsFilter::setAllowedOrigins(config.timeline.cors_origins);

        // Resolve static files path (absolute)
        std::string static_path = config.timeline.static_files_path;
        if (!fs::path(static_path).is_absolute()) {
            static_path = (fs::path(config_path).parent_path() / static_path).string();
        }
        std::string static_abs = fs::absolute(static_path).string();

        spdlog::info("Static files: {}", static_abs);
        spdlog::info("Events dir:   {}", config.timeline.events_dir);
        spdlog::info("Snapshots:    {}", config.timeline.snapshots_dir);
        spdlog::info("Detection:    {}", config.timeline.detection_service_url);

        auto& app = drogon::app();
        app.setLogLevel(trantor::Logger::kWarn);
        app.addListener(config.timeline.host, config.timeline.port);
        app.setThreadNum(4);
        app.setMaxConnectionNum(100);

        // -------------------------------------------------------------------
        // SPA catch-all handler — serves Angular for any path not claimed by
        // the HttpController routes (/api/*, /health, /events/*, /snapshots/*).
        //
        // Priority in Drogon: HttpController ADD_METHOD_TO > regex handlers.
        // So API routes always win; this only fires for unmatched paths.
        //
        // HA Ingress support: reads X-Ingress-Path header and injects it as
        // <base href> in index.html, so Angular resolves URLs correctly.
        //
        // Request patterns served here:
        //   /                          → index.html (HA ingress root)
        //   /patio, /side_window, ...  → index.html (Angular client routes)
        //   /main-ABC.js               → static asset from dist/browser/
        //   /styles-XYZ.css            → static asset
        // -------------------------------------------------------------------
        const std::string captured_static = static_abs;

        app.registerHandlerViaRegex(
            R"(/(.*))",
            [captured_static](const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& cb,
                               const std::string& sub_path) {

                // sub_path is everything after the leading /
                // Try to serve as a static asset first (JS, CSS, fonts, favicons, etc.)
                if (!sub_path.empty()) {
                    auto file_path = fs::path(captured_static) / sub_path;
                    if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
                        cb(drogon::HttpResponse::newFileResponse(file_path.string()));
                        return;
                    }
                }

                // Not a static file — serve index.html (Angular client-side routing)
                // Inject X-Ingress-Path as <base href> for HA ingress support
                auto ingress_path = std::string(req->getHeader("X-Ingress-Path"));
                auto html = load_index_html(captured_static, ingress_path);

                if (html.empty()) {
                    cb(drogon::HttpResponse::newNotFoundResponse());
                    return;
                }

                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k200OK);
                resp->setContentTypeCode(drogon::CT_TEXT_HTML);
                resp->setBody(std::move(html));
                cb(resp);
            },
            {drogon::Get}
        );

        // Global CORS headers for all responses
        app.registerPostHandlingAdvice(
            [](const drogon::HttpRequestPtr& req, const drogon::HttpResponsePtr& resp) {
                auto origin = std::string(req->getHeader("Origin"));
                std::string allow_origin = origin.empty() ? "*" : origin;
                resp->addHeader("Access-Control-Allow-Origin", allow_origin);
                resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                resp->addHeader("Access-Control-Allow-Headers",
                                "Content-Type, Authorization, Accept");
                if (allow_origin != "*") {
                    resp->addHeader("Vary", "Origin");
                }
            }
        );

        spdlog::info("Listening on {}:{}", config.timeline.host, config.timeline.port);
        spdlog::info("Angular UI: http://{}:{}/", config.timeline.host, config.timeline.port);

        app.run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
