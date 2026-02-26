#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <algorithm>

using json = nlohmann::json;

// Controller logic unit tests (no HTTP server required)

namespace {

// Mirrors MediaController::isValidFilename logic
bool isValidFilename(const std::string& filename) {
    if (filename.empty()) return false;
    if (filename.find("..") != std::string::npos) return false;
    if (filename.find('/') != std::string::npos) return false;
    if (filename.find('\\') != std::string::npos) return false;
    return std::all_of(filename.begin(), filename.end(), [](char c) {
        return std::isalnum(c) || c == '-' || c == '_' || c == '.';
    });
}

} // anonymous namespace

TEST_CASE("Filename validation prevents path traversal", "[media]") {
    // Valid filenames
    CHECK(isValidFilename("patio_20260225_103000.mp4"));
    CHECK(isValidFilename("front_door_20260225_103000.jpg"));
    CHECK(isValidFilename("test-file.mp4"));
    CHECK(isValidFilename("file123.txt"));

    // Invalid: path traversal
    CHECK_FALSE(isValidFilename("../etc/passwd"));
    CHECK_FALSE(isValidFilename("..%2f..%2fetc%2fpasswd"));
    CHECK_FALSE(isValidFilename("../../secret.txt"));

    // Invalid: directory separators
    CHECK_FALSE(isValidFilename("subdir/file.mp4"));
    CHECK_FALSE(isValidFilename("subdir\\file.mp4"));

    // Invalid: empty
    CHECK_FALSE(isValidFilename(""));

    // Invalid: special characters
    CHECK_FALSE(isValidFilename("file;rm -rf.mp4"));
    CHECK_FALSE(isValidFilename("file$(cmd).mp4"));
    CHECK_FALSE(isValidFilename("file`cmd`.mp4"));
}

TEST_CASE("Events response JSON structure", "[api]") {
    // Simulate the response structure from GET /api/events
    json events = json::array();
    events.push_back({
        {"event_id", "patio_20260225_103000"},
        {"camera_id", "patio"},
        {"camera_name", "Patio"},
        {"started_at", "2026-02-25T10:30:00"},
        {"total_detections", 5},
        {"status", "completed"}
    });

    json response;
    response["events"] = events;
    response["count"] = events.size();

    CHECK(response["count"] == 1);
    CHECK(response["events"][0]["camera_id"] == "patio");
}

TEST_CASE("Health endpoint JSON structure", "[api]") {
    json health;
    health["service"] = "yolo-timeline";
    health["status"] = "healthy";
    health["timestamp"] = "2026-02-25T10:30:00";
    health["database"] = {
        {"total_connections", 4},
        {"available_connections", 3},
        {"in_use_connections", 1}
    };

    CHECK(health["service"] == "yolo-timeline");
    CHECK(health["status"] == "healthy");
    CHECK(health["database"]["total_connections"] == 4);
}

TEST_CASE("Limit parameter validation", "[api]") {
    // Simulate limit parameter parsing
    auto parse_limit = [](const std::string& str) -> int {
        int limit = 100;
        try {
            limit = std::stoi(str);
            if (limit <= 0 || limit > 1000) limit = 100;
        } catch (...) {
            limit = 100;
        }
        return limit;
    };

    CHECK(parse_limit("50") == 50);
    CHECK(parse_limit("100") == 100);
    CHECK(parse_limit("1000") == 1000);
    CHECK(parse_limit("0") == 100);       // Invalid: too low
    CHECK(parse_limit("-1") == 100);      // Invalid: negative
    CHECK(parse_limit("1001") == 100);    // Invalid: too high
    CHECK(parse_limit("abc") == 100);     // Invalid: not a number
    CHECK(parse_limit("") == 100);        // Invalid: empty
}
