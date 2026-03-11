#pragma once

#include <drogon/HttpController.h>
#include <memory>
#include "db_pool.h"

namespace yolo {

/// REST API controller for the Angular Timeline UI.
/// Ports the API endpoints from api_server.py that serve the frontend.
class UiApiController : public drogon::HttpController<UiApiController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(UiApiController::getEvents, "/api/events", drogon::Get, "yolo::CorsFilter");
    ADD_METHOD_TO(UiApiController::getEventDetail, "/api/events/{event_id}", drogon::Get, "yolo::CorsFilter");
    ADD_METHOD_TO(UiApiController::getTimeline, "/api/timeline", drogon::Get, "yolo::CorsFilter");
    ADD_METHOD_TO(UiApiController::getCamerasStatus, "/api/cameras/status", drogon::Get, "yolo::CorsFilter");
    ADD_METHOD_TO(UiApiController::getCameraSnapshot, "/api/cameras/{camera_id}/snapshot", drogon::Get, "yolo::CorsFilter");
    ADD_METHOD_TO(UiApiController::searchEvents, "/api/search", drogon::Get, "yolo::CorsFilter");
    ADD_METHOD_TO(UiApiController::getPeriodicSnapshots, "/api/snapshots", drogon::Get, "yolo::CorsFilter");
    ADD_METHOD_TO(UiApiController::getCameraPaused, "/api/cameras/{camera_id}/paused", drogon::Get, "yolo::CorsFilter");
    ADD_METHOD_TO(UiApiController::setCameraPaused, "/api/cameras/{camera_id}/paused", drogon::Post, "yolo::CorsFilter");
    ADD_METHOD_TO(UiApiController::getHealth, "/health", drogon::Get);
    METHOD_LIST_END

    /// GET /api/events?camera_id=X&start=...&end=...&limit=100
    void getEvents(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /// GET /api/events/{event_id}
    void getEventDetail(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                        const std::string& event_id);

    /// GET /api/timeline?camera_id=X&date=YYYY-MM-DD
    void getTimeline(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /// GET /api/cameras/status
    void getCamerasStatus(const drogon::HttpRequestPtr& req,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /// GET /api/cameras/{camera_id}/snapshot — proxy to detection service
    void getCameraSnapshot(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& camera_id);

    /// GET /api/search?q=...&classes=...&camera_id=...&start=...&end=...&limit=50&mode=auto
    void searchEvents(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /// GET /api/snapshots?camera_id=X&date=YYYY-MM-DD
    void getPeriodicSnapshots(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /// GET /api/cameras/{camera_id}/paused — proxy to detection service
    void getCameraPaused(const drogon::HttpRequestPtr& req,
                         std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                         const std::string& camera_id);

    /// POST /api/cameras/{camera_id}/paused — proxy to detection service
    void setCameraPaused(const drogon::HttpRequestPtr& req,
                         std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                         const std::string& camera_id);

    /// GET /health
    void getHealth(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /// Set the shared database pool (called once at startup)
    static void setDbPool(std::shared_ptr<DbPool> pool);

    /// Set the detection service URL for snapshot proxying
    static void setDetectionServiceUrl(std::string url);

    /// Set the Ollama URL for query-time embeddings
    static void setOllamaUrl(std::string url);

private:
    static inline std::shared_ptr<DbPool> db_pool_;
    static inline std::string detection_service_url_;
    static inline std::string ollama_url_;
};

} // namespace yolo
