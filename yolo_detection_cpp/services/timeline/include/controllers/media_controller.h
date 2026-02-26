#pragma once

#include <drogon/HttpController.h>
#include <string>

namespace yolo {

/// Controller for serving media files (recordings and snapshots).
/// Also handles Angular SPA static file serving and routing fallback.
class MediaController : public drogon::HttpController<MediaController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MediaController::serveEvent, "/events/{filename}", drogon::Get, "yolo::CorsFilter");
    ADD_METHOD_TO(MediaController::serveSnapshot, "/snapshots/{filename}", drogon::Get, "yolo::CorsFilter");
    METHOD_LIST_END

    /// GET /events/{filename} — serve MP4 recording files
    void serveEvent(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    const std::string& filename);

    /// GET /snapshots/{filename} — serve JPEG snapshot files
    void serveSnapshot(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                       const std::string& filename);

    /// Set media directories (called once at startup)
    static void setEventsDir(std::string dir);
    static void setSnapshotsDir(std::string dir);

private:
    /// Validate filename to prevent path traversal attacks
    static bool isValidFilename(const std::string& filename);

    /// Serve a file from a directory with content type
    static void serveFile(const std::string& dir,
                          const std::string& filename,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    static inline std::string events_dir_;
    static inline std::string snapshots_dir_;
};

} // namespace yolo
