#include "time_utils.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <regex>

namespace yolo::time_utils {

std::string to_iso8601(const TimePoint& tp) {
    auto time_t_val = Clock::to_time_t(tp);
    auto duration = tp.time_since_epoch();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;

    std::tm tm_val{};
    gmtime_r(&time_t_val, &tm_val);

    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%dT%H:%M:%S");
    if (micros > 0) {
        oss << '.' << std::setfill('0') << std::setw(6) << micros;
    }
    return oss.str();
}

std::optional<TimePoint> from_iso8601(const std::string& str) {
    if (str.empty()) return std::nullopt;

    std::tm tm_val{};
    int microseconds = 0;

    // Try full datetime with optional fractional seconds
    // Format: YYYY-MM-DDTHH:MM:SS[.ffffff]
    std::istringstream iss(str);
    iss >> std::get_time(&tm_val, "%Y-%m-%dT%H:%M:%S");

    if (iss.fail()) {
        // Try date-only format: YYYY-MM-DD
        iss.clear();
        iss.str(str);
        iss >> std::get_time(&tm_val, "%Y-%m-%d");
        if (iss.fail()) {
            return std::nullopt;
        }
    } else {
        // Check for fractional seconds
        char dot;
        if (iss.peek() == '.') {
            iss.get(dot);
            std::string frac;
            while (iss.peek() != EOF && std::isdigit(iss.peek())) {
                frac += static_cast<char>(iss.get());
            }
            // Pad or truncate to 6 digits (microseconds)
            while (frac.size() < 6) frac += '0';
            frac = frac.substr(0, 6);
            microseconds = std::stoi(frac);
        }
    }

    auto time_t_val = timegm(&tm_val);
    if (time_t_val == -1) return std::nullopt;

    auto tp = Clock::from_time_t(time_t_val);
    tp += std::chrono::microseconds(microseconds);
    return tp;
}

std::optional<TimePoint> from_date_string(const std::string& str) {
    if (str.empty()) return std::nullopt;

    std::tm tm_val{};
    std::istringstream iss(str);
    iss >> std::get_time(&tm_val, "%Y-%m-%d");
    if (iss.fail()) return std::nullopt;

    auto time_t_val = timegm(&tm_val);
    if (time_t_val == -1) return std::nullopt;

    return Clock::from_time_t(time_t_val);
}

TimePoint start_of_day(const TimePoint& tp) {
    auto time_t_val = Clock::to_time_t(tp);
    std::tm tm_val{};
    gmtime_r(&time_t_val, &tm_val);
    tm_val.tm_hour = 0;
    tm_val.tm_min = 0;
    tm_val.tm_sec = 0;
    return Clock::from_time_t(timegm(&tm_val));
}

TimePoint end_of_day(const TimePoint& tp) {
    auto sod = start_of_day(tp);
    return sod + std::chrono::hours(24);
}

std::string now_iso8601() {
    return to_iso8601(Clock::now());
}

std::string to_date_string(const TimePoint& tp) {
    auto time_t_val = Clock::to_time_t(tp);
    std::tm tm_val{};
    gmtime_r(&time_t_val, &tm_val);

    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%d");
    return oss.str();
}

std::string pg_timestamp_to_iso8601(const std::string& pg_ts) {
    if (pg_ts.empty()) return "";

    // PostgreSQL timestamps use space separator: "2026-02-25 10:30:00.123456"
    // ISO 8601 uses T separator: "2026-02-25T10:30:00.123456"
    std::string result = pg_ts;
    auto space_pos = result.find(' ');
    if (space_pos != std::string::npos && space_pos == 10) {
        result[space_pos] = 'T';
    }
    return result;
}

} // namespace yolo::time_utils
