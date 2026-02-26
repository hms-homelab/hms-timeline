#include "controllers/media_controller.h"
#include "http_utils.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <algorithm>

using namespace drogon;
using nlohmann::json;
namespace fs = std::filesystem;

namespace yolo {

void MediaController::setEventsDir(std::string dir) {
    events_dir_ = std::move(dir);
}

void MediaController::setSnapshotsDir(std::string dir) {
    snapshots_dir_ = std::move(dir);
}

bool MediaController::isValidFilename(const std::string& filename) {
    // Prevent path traversal: no "..", no "/", no "\"
    if (filename.empty()) return false;
    if (filename.find("..") != std::string::npos) return false;
    if (filename.find('/') != std::string::npos) return false;
    if (filename.find('\\') != std::string::npos) return false;
    // Only allow alphanumeric, dash, underscore, dot
    return std::all_of(filename.begin(), filename.end(), [](char c) {
        return std::isalnum(c) || c == '-' || c == '_' || c == '.';
    });
}

void MediaController::serveFile(const std::string& dir,
                                 const std::string& filename,
                                 std::function<void(const HttpResponsePtr&)>&& callback) {
    if (!isValidFilename(filename)) {
        spdlog::warn("Rejected invalid filename: {}", filename);
        callback(makeJsonResponse(json{{"error", "Invalid filename"}}, k400BadRequest));
        return;
    }

    auto filepath = fs::path(dir) / filename;

    if (!fs::exists(filepath)) {
        callback(makeJsonResponse(json{{"error", "File not found"}}, k404NotFound));
        return;
    }

    // Use Drogon's built-in file response for efficient serving (supports range requests)
    auto resp = HttpResponse::newFileResponse(filepath.string());
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}

void MediaController::serveEvent(const HttpRequestPtr& req,
                                  std::function<void(const HttpResponsePtr&)>&& callback,
                                  const std::string& filename) {
    spdlog::debug("GET /events/{}", filename);
    serveFile(events_dir_, filename, std::move(callback));
}

void MediaController::serveSnapshot(const HttpRequestPtr& req,
                                     std::function<void(const HttpResponsePtr&)>&& callback,
                                     const std::string& filename) {
    spdlog::debug("GET /snapshots/{}", filename);
    serveFile(snapshots_dir_, filename, std::move(callback));
}

} // namespace yolo
