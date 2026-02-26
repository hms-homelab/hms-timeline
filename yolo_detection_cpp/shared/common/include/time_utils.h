#pragma once

#include <string>
#include <chrono>
#include <optional>

namespace yolo {

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;

namespace time_utils {

/// Format a time point as ISO 8601 string (e.g., "2026-02-25T10:30:00.123456")
std::string to_iso8601(const TimePoint& tp);

/// Parse an ISO 8601 string to a time point
/// Accepts formats: "2026-02-25T10:30:00", "2026-02-25T10:30:00.123456",
///                   "2026-02-25" (midnight)
std::optional<TimePoint> from_iso8601(const std::string& str);

/// Parse a date string "YYYY-MM-DD" to a time point at midnight
std::optional<TimePoint> from_date_string(const std::string& str);

/// Get start of day (midnight) for a given time point
TimePoint start_of_day(const TimePoint& tp);

/// Get end of day (next midnight) for a given time point
TimePoint end_of_day(const TimePoint& tp);

/// Get current time as ISO 8601 string
std::string now_iso8601();

/// Format a time point as "YYYY-MM-DD"
std::string to_date_string(const TimePoint& tp);

/// Convert a PostgreSQL timestamp string to ISO 8601
/// Handles formats like "2026-02-25 10:30:00.123456" (space separator)
std::string pg_timestamp_to_iso8601(const std::string& pg_ts);

} // namespace time_utils
} // namespace yolo
