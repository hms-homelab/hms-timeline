#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <sstream>
#include <vector>
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

// ────────────────────────────────────────────────────────────────────
// Search endpoint parameter parsing + response structure tests
// ────────────────────────────────────────────────────────────────────

TEST_CASE("Search limit parameter validation (capped at 200)", "[api][search]") {
    // Search endpoint uses stricter limit than events endpoint
    auto parse_search_limit = [](const std::string& str) -> int {
        int limit = 50;
        try {
            limit = std::stoi(str);
        } catch (...) {
            return 50;
        }
        if (limit <= 0 || limit > 200) limit = 50;
        return limit;
    };

    CHECK(parse_search_limit("25") == 25);
    CHECK(parse_search_limit("50") == 50);
    CHECK(parse_search_limit("200") == 200);
    CHECK(parse_search_limit("0") == 50);     // Invalid: too low
    CHECK(parse_search_limit("-5") == 50);    // Invalid: negative
    CHECK(parse_search_limit("201") == 50);   // Invalid: too high (capped at 200)
    CHECK(parse_search_limit("abc") == 50);   // Invalid: NaN
    CHECK(parse_search_limit("") == 50);      // Invalid: empty
}

TEST_CASE("Search mode parameter validation", "[api][search]") {
    auto is_valid_mode = [](const std::string& mode) -> bool {
        return mode == "auto" || mode == "fts" || mode == "semantic";
    };

    CHECK(is_valid_mode("auto"));
    CHECK(is_valid_mode("fts"));
    CHECK(is_valid_mode("semantic"));
    CHECK_FALSE(is_valid_mode(""));
    CHECK_FALSE(is_valid_mode("hybrid"));
    CHECK_FALSE(is_valid_mode("keyword"));
}

TEST_CASE("Class filter parsing from comma-separated string", "[api][search]") {
    // Mirrors the parsing logic in searchEvents handler
    auto parse_classes = [](const std::string& classes_str) -> std::vector<std::string> {
        std::vector<std::string> result;
        std::istringstream iss(classes_str);
        std::string cls;
        while (std::getline(iss, cls, ',')) {
            auto start = cls.find_first_not_of(' ');
            if (start != std::string::npos) {
                result.push_back(cls.substr(start));
            }
        }
        return result;
    };

    SECTION("Single class") {
        auto classes = parse_classes("person");
        REQUIRE(classes.size() == 1);
        CHECK(classes[0] == "person");
    }

    SECTION("Multiple classes") {
        auto classes = parse_classes("person,dog,cat");
        REQUIRE(classes.size() == 3);
        CHECK(classes[0] == "person");
        CHECK(classes[1] == "dog");
        CHECK(classes[2] == "cat");
    }

    SECTION("With spaces") {
        auto classes = parse_classes("person, dog, cat");
        REQUIRE(classes.size() == 3);
        CHECK(classes[0] == "person");
        CHECK(classes[1] == "dog");
        CHECK(classes[2] == "cat");
    }

    SECTION("Empty string") {
        auto classes = parse_classes("");
        CHECK(classes.empty());
    }
}

TEST_CASE("Search response JSON structure", "[api][search]") {
    json response;
    response["events"] = json::array({
        {
            {"type", "event"},
            {"id", "patio_20260304_103000"},
            {"camera_id", "patio"},
            {"camera_name", "Patio"},
            {"timestamp", "2026-03-04T10:30:00Z"},
            {"recording_url", "patio_20260304_103000.mp4"},
            {"snapshot_url", "patio_20260304_103000.jpg"},
            {"total_detections", 3},
            {"duration_seconds", 12.5},
            {"detected_classes", "person, dog"},
            {"ai_context", "A person walking with a dog"},
            {"rank", 0.85}
        },
        {
            {"type", "snapshot"},
            {"id", "42"},
            {"camera_id", "patio"},
            {"camera_name", "patio"},
            {"timestamp", "2026-03-04T14:30:00Z"},
            {"recording_url", nullptr},
            {"snapshot_url", "patio_periodic_20260304_143000.jpg"},
            {"total_detections", 0},
            {"detected_classes", nullptr},
            {"ai_context", "Empty patio on a sunny day"},
            {"rank", 0.72}
        }
    });
    response["count"] = 2;
    response["search_mode"] = "fts";
    response["query"] = "person walking";

    CHECK(response["count"] == 2);
    CHECK(response["search_mode"] == "fts");
    CHECK(response["query"] == "person walking");

    // Event results have recording_url
    CHECK_FALSE(response["events"][0]["recording_url"].is_null());

    // Snapshot results have no recording
    CHECK(response["events"][1]["recording_url"].is_null());
    CHECK(response["events"][1]["type"] == "snapshot");
}

TEST_CASE("Snapshots response JSON structure", "[api][search]") {
    json response;
    response["snapshots"] = json::array({
        {
            {"type", "snapshot"},
            {"snapshot_id", 1},
            {"camera_id", "patio"},
            {"captured_at", "2026-03-04T10:00:00Z"},
            {"snapshot_url", "patio_periodic_20260304_100000.jpg"},
            {"thumbnail_url", "patio_periodic_20260304_100000_thumb.jpg"},
            {"ai_context", "Sunny patio, garden furniture visible"},
            {"is_valid", true}
        }
    });
    response["count"] = 1;

    CHECK(response.contains("snapshots"));
    CHECK(response.contains("count"));
    CHECK(response["snapshots"].is_array());
    CHECK(response["count"] == 1);

    auto& snap = response["snapshots"][0];
    CHECK(snap["type"] == "snapshot");
    CHECK(snap["snapshot_id"] == 1);
    CHECK(snap.contains("captured_at"));
    CHECK(snap.contains("snapshot_url"));
}

TEST_CASE("Search auto-mode fallback logic", "[api][search]") {
    // Simulate the auto-mode decision tree
    auto should_try_semantic = [](const std::string& mode, int fts_count) -> bool {
        if (mode != "auto") return false;
        return fts_count < 3;
    };

    CHECK(should_try_semantic("auto", 0));   // No FTS results → try semantic
    CHECK(should_try_semantic("auto", 1));   // 1 result → try semantic
    CHECK(should_try_semantic("auto", 2));   // 2 results → try semantic
    CHECK_FALSE(should_try_semantic("auto", 3));   // 3+ results → FTS enough
    CHECK_FALSE(should_try_semantic("auto", 10));  // Many results → FTS enough
    CHECK_FALSE(should_try_semantic("fts", 0));    // FTS-only mode → no fallback
    CHECK_FALSE(should_try_semantic("semantic", 0)); // Semantic-only → no fallback
}

TEST_CASE("Filename validation for periodic snapshot filenames", "[media][search]") {
    // Periodic snapshots follow a specific naming convention
    CHECK(isValidFilename("patio_periodic_20260304_143000.jpg"));
    CHECK(isValidFilename("front_door_periodic_20260304_090000.jpg"));
    CHECK(isValidFilename("side_window_periodic_20260304_120000_thumb.jpg"));

    // Standard event filenames still valid
    CHECK(isValidFilename("patio_20260304_143000.mp4"));
    CHECK(isValidFilename("patio_20260304_143000.jpg"));

    // Invalid: attempts to escape
    CHECK_FALSE(isValidFilename("../patio_periodic_20260304.jpg"));
    CHECK_FALSE(isValidFilename("patio_periodic/20260304.jpg"));
}
