#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "db_pool.h"
#include "config_manager.h"

namespace yolo {

using json = nlohmann::json;

namespace api_queries {

/// Query all detection events with optional filters.
/// Ports: api_queries.py:get_all_events()
json get_all_events(
    DbPool& db,
    const std::optional<std::string>& start_date = std::nullopt,
    const std::optional<std::string>& end_date = std::nullopt,
    const std::optional<std::string>& camera_id = std::nullopt,
    int limit = 100
);

/// Query single event with all detection details.
/// Ports: api_queries.py:get_event_detail()
json get_event_detail(DbPool& db, const std::string& event_id);

/// Get aggregated timeline data for a specific camera and date.
/// Returns hourly event counts for timeline rendering.
/// Ports: api_queries.py:get_timeline_data()
json get_timeline_data(
    DbPool& db,
    const std::string& camera_id,
    const std::string& date  // "YYYY-MM-DD"
);

/// Get the timestamp of the most recent event for a camera.
/// Ports: api_queries.py:get_camera_last_event()
std::optional<std::string> get_camera_last_event(
    DbPool& db,
    const std::string& camera_id
);

/// Get camera status list with last event times.
/// Used by GET /api/cameras/status
json get_cameras_status(
    DbPool& db,
    const std::unordered_map<std::string, CameraConfig>& cameras
);

} // namespace api_queries
} // namespace yolo
