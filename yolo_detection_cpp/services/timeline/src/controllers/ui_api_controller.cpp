#include "controllers/ui_api_controller.h"
#include "api_queries.h"
#include "config_manager.h"
#include "time_utils.h"
#include "http_utils.h"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

using namespace drogon;

namespace yolo {

void UiApiController::setDbPool(std::shared_ptr<DbPool> pool) {
    db_pool_ = std::move(pool);
}

void UiApiController::setDetectionServiceUrl(std::string url) {
    detection_service_url_ = std::move(url);
}

void UiApiController::getEvents(const HttpRequestPtr& req,
                                 std::function<void(const HttpResponsePtr&)>&& callback) {
    auto camera_id_param = req->getOptionalParameter<std::string>("camera_id");
    auto start_param = req->getOptionalParameter<std::string>("start");
    auto end_param = req->getOptionalParameter<std::string>("end");
    auto limit_str = req->getOptionalParameter<std::string>("limit");
    // Match Python default: only_with_recordings=true
    auto only_with_recordings_str = req->getOptionalParameter<std::string>("only_with_recordings");
    bool only_with_recordings = true;
    if (only_with_recordings_str && (*only_with_recordings_str == "false" || *only_with_recordings_str == "0")) {
        only_with_recordings = false;
    }

    int limit = 100;
    if (limit_str) {
        try {
            limit = std::stoi(*limit_str);
            if (limit <= 0 || limit > 1000) limit = 100;
        } catch (...) {}
    }

    spdlog::debug("GET /api/events camera_id={} limit={} only_with_recordings={}",
                  camera_id_param.value_or("all"), limit, only_with_recordings);

    // Query 3x more to account for recording file filtering (matches Python behaviour)
    auto raw_events = api_queries::get_all_events(
        *db_pool_, start_param, end_param, camera_id_param, limit * 3);

    // Filter to events whose recording file exists on disk (Python: only_with_recordings=True)
    nlohmann::json events = nlohmann::json::array();
    if (only_with_recordings) {
        const auto& events_dir = ConfigManager::get().timeline.events_dir;
        for (const auto& event : raw_events) {
            if (events.size() >= static_cast<size_t>(limit)) break;
            auto recording_url = event.value("recording_url", "");
            if (recording_url.empty()) continue;
            // Extract filename from URL (last path component)
            auto slash = recording_url.rfind('/');
            std::string filename = (slash != std::string::npos)
                                   ? recording_url.substr(slash + 1)
                                   : recording_url;
            if (!filename.empty() &&
                std::filesystem::exists(std::filesystem::path(events_dir) / filename)) {
                events.push_back(event);
            }
        }
    } else {
        for (const auto& event : raw_events) {
            if (events.size() >= static_cast<size_t>(limit)) break;
            events.push_back(event);
        }
    }

    nlohmann::json response;
    response["events"] = events;
    response["count"] = static_cast<int>(events.size());

    callback(makeJsonResponse(response));
}

void UiApiController::getEventDetail(const HttpRequestPtr& req,
                                      std::function<void(const HttpResponsePtr&)>&& callback,
                                      const std::string& event_id) {
    spdlog::debug("GET /api/events/{}", event_id);

    auto detail = api_queries::get_event_detail(*db_pool_, event_id);

    if (detail.is_null()) {
        callback(makeJsonResponse(
            nlohmann::json{{"error", "Event not found"}, {"event_id", event_id}},
            k404NotFound));
        return;
    }

    callback(makeJsonResponse(detail));
}

void UiApiController::getTimeline(const HttpRequestPtr& req,
                                   std::function<void(const HttpResponsePtr&)>&& callback) {
    auto camera_id = req->getOptionalParameter<std::string>("camera_id");
    auto date = req->getOptionalParameter<std::string>("date");

    if (!camera_id) {
        callback(makeJsonResponse(
            nlohmann::json{{"error", "camera_id parameter is required"}},
            k400BadRequest));
        return;
    }

    std::string date_str = date.value_or(
        time_utils::to_date_string(std::chrono::system_clock::now()));

    spdlog::debug("GET /api/timeline camera_id={} date={}", *camera_id, date_str);

    auto timeline = api_queries::get_timeline_data(*db_pool_, *camera_id, date_str);
    callback(makeJsonResponse(timeline));
}

void UiApiController::getCamerasStatus(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) {
    spdlog::debug("GET /api/cameras/status");

    const auto& config = ConfigManager::get();
    auto cameras = api_queries::get_cameras_status(*db_pool_, config.cameras);
    // Match Python response shape: {"cameras": [...]}
    callback(makeJsonResponse(nlohmann::json{{"cameras", cameras}}));
}

void UiApiController::getCameraSnapshot(const HttpRequestPtr& req,
                                         std::function<void(const HttpResponsePtr&)>&& callback,
                                         const std::string& camera_id) {
    if (detection_service_url_.empty()) {
        callback(makeJsonResponse(
            nlohmann::json{{"error", "Detection service URL not configured"}},
            k503ServiceUnavailable));
        return;
    }

    // Parse host and port from detection_service_url (e.g. "http://localhost:8000")
    std::string url = detection_service_url_;
    if (url.substr(0, 7) == "http://") url = url.substr(7);
    std::string host = url;
    std::string port = "80";
    auto colon = url.rfind(':');
    if (colon != std::string::npos) {
        host = url.substr(0, colon);
        port = url.substr(colon + 1);
    }

    std::string path = "/api/cameras/" + camera_id + "/snapshot";
    spdlog::debug("Proxying snapshot {} {}:{}{}", camera_id, host, port, path);

    // Simple blocking HTTP GET — runs in Drogon's worker thread pool, not the IO loop
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family   = AF_INET;    // Force IPv4 — avoids ::1 resolution when server is 0.0.0.0
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0 || !res) {
        spdlog::warn("Snapshot proxy: getaddrinfo failed for {}", host);
        callback(makeJsonResponse(nlohmann::json{{"error", "Detection service unavailable"}}, k502BadGateway));
        return;
    }

    int fd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    struct timeval tv{4, 0};  // 4s timeout
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (fd < 0 || ::connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
        freeaddrinfo(res);
        if (fd >= 0) ::close(fd);
        spdlog::warn("Snapshot proxy: connect failed for {}:{}", host, port);
        callback(makeJsonResponse(nlohmann::json{{"error", "Detection service unavailable"}}, k502BadGateway));
        return;
    }
    freeaddrinfo(res);

    // Send HTTP/1.0 request (no keep-alive, server closes after response)
    std::string request = "GET " + path + " HTTP/1.0\r\n"
                          "Host: " + host + "\r\n"
                          "Connection: close\r\n\r\n";
    ::send(fd, request.c_str(), request.size(), 0);

    // Read full response
    std::string raw;
    raw.reserve(128 * 1024);
    char buf[8192];
    ssize_t n;
    while ((n = ::recv(fd, buf, sizeof(buf), 0)) > 0) {
        raw.append(buf, n);
    }
    ::close(fd);

    if (raw.empty()) {
        spdlog::warn("Snapshot proxy: empty response from detection service");
        callback(makeJsonResponse(nlohmann::json{{"error", "Empty response from detection service"}}, k502BadGateway));
        return;
    }

    // Split HTTP headers from body
    auto header_end = raw.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        callback(makeJsonResponse(nlohmann::json{{"error", "Malformed HTTP response"}}, k502BadGateway));
        return;
    }

    std::string headers = raw.substr(0, header_end);
    std::string body    = raw.substr(header_end + 4);

    // Parse status code from first line
    int status_code = 502;
    if (headers.size() >= 12) {
        try { status_code = std::stoi(headers.substr(9, 3)); } catch (...) {}
    }

    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(static_cast<HttpStatusCode>(status_code));
    resp->setBody(std::move(body));

    // Extract Content-Type header
    auto ct_pos = headers.find("Content-Type:");
    if (ct_pos != std::string::npos) {
        auto ct_end = headers.find("\r\n", ct_pos);
        std::string ct = headers.substr(ct_pos + 13, ct_end - ct_pos - 13);
        // trim leading space
        auto first = ct.find_first_not_of(' ');
        if (first != std::string::npos) ct = ct.substr(first);
        resp->addHeader("Content-Type", ct);
    } else {
        resp->setContentTypeCode(CT_IMAGE_JPG);
    }

    spdlog::debug("Snapshot proxy: {} bytes for {}", body.size(), camera_id);
    callback(resp);
}

void UiApiController::getHealth(const HttpRequestPtr& req,
                                 std::function<void(const HttpResponsePtr&)>&& callback) {
    nlohmann::json health;
    health["service"] = "yolo-timeline";
    health["status"] = "healthy";
    health["timestamp"] = time_utils::now_iso8601();

    if (db_pool_) {
        auto stats = db_pool_->stats();
        health["database"] = {
            {"total_connections", stats.total_connections},
            {"available_connections", stats.available_connections},
            {"in_use_connections", stats.in_use_connections},
        };
    }

    callback(makeJsonResponse(health));
}

} // namespace yolo
