#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "config_manager.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

// Helper to create a temporary config file
struct TempConfig {
    std::string path;

    TempConfig(const std::string& content) {
        path = fs::temp_directory_path() / "test_config.yaml";
        std::ofstream f(path);
        f << content;
    }

    ~TempConfig() {
        fs::remove(path);
    }
};

} // anonymous namespace

TEST_CASE("ConfigManager loads camera configuration", "[config]") {
    TempConfig cfg(R"(
cameras:
  patio:
    name: "Patio"
    rtsp_url: "rtsp://admin:pass@192.168.2.59:554/stream"
    enabled: true
    classes: [person, dog, cat]
    confidence_threshold: 0.6
    immediate_notification_confidence: 0.75
  front_door:
    name: "Front Door"
    rtsp_url: "rtsp://admin:pass@192.168.2.43:554/stream"
    enabled: false
)");

    auto config = yolo::ConfigManager::load(cfg.path);

    REQUIRE(config.cameras.size() == 2);

    SECTION("Camera properties are parsed correctly") {
        auto& patio = config.cameras.at("patio");
        CHECK(patio.name == "Patio");
        CHECK(patio.rtsp_url == "rtsp://admin:pass@192.168.2.59:554/stream");
        CHECK(patio.enabled == true);
        CHECK(patio.classes.size() == 3);
        CHECK(patio.classes[0] == "person");
        CHECK(patio.classes[1] == "dog");
        CHECK(patio.classes[2] == "cat");
        CHECK(patio.confidence_threshold == 0.6);
        CHECK(patio.immediate_notification_confidence == 0.75);
    }

    SECTION("Disabled camera is loaded") {
        auto& front = config.cameras.at("front_door");
        CHECK(front.enabled == false);
    }
}

TEST_CASE("ConfigManager uses defaults for missing sections", "[config]") {
    TempConfig cfg(R"(
cameras:
  test:
    name: "Test"
)");

    auto config = yolo::ConfigManager::load(cfg.path);

    CHECK(config.buffer.preroll_seconds == 5);
    CHECK(config.buffer.fps == 15);
    CHECK(config.detection.confidence_threshold == 0.5);
    CHECK(config.mqtt.port == 1883);
    CHECK(config.api.port == 8000);
    CHECK(config.database.port == 5432);
    CHECK(config.timeline.port == 8080);
    CHECK(config.logging.level == "info");
}

TEST_CASE("ConfigManager loads database settings", "[config]") {
    TempConfig cfg(R"(
cameras: {}
database:
  host: "10.0.0.1"
  port: 5433
  user: "testuser"
  password: "testpass"
  database: "testdb"
  pool_size: 8
)");

    auto config = yolo::ConfigManager::load(cfg.path);

    CHECK(config.database.host == "10.0.0.1");
    CHECK(config.database.port == 5433);
    CHECK(config.database.user == "testuser");
    CHECK(config.database.password == "testpass");
    CHECK(config.database.database == "testdb");
    CHECK(config.database.pool_size == 8);
}

TEST_CASE("ConfigManager loads timeline settings", "[config]") {
    TempConfig cfg(R"(
cameras: {}
timeline:
  host: "127.0.0.1"
  port: 9090
  static_files_path: "/var/www/html"
  events_dir: "/data/events"
  snapshots_dir: "/data/snapshots"
  detection_service_url: "http://detect:8000"
  cors_origins:
    - "http://localhost:4200"
    - "http://localhost:3000"
)");

    auto config = yolo::ConfigManager::load(cfg.path);

    CHECK(config.timeline.host == "127.0.0.1");
    CHECK(config.timeline.port == 9090);
    CHECK(config.timeline.static_files_path == "/var/www/html");
    CHECK(config.timeline.events_dir == "/data/events");
    CHECK(config.timeline.snapshots_dir == "/data/snapshots");
    CHECK(config.timeline.detection_service_url == "http://detect:8000");
    REQUIRE(config.timeline.cors_origins.size() == 2);
    CHECK(config.timeline.cors_origins[0] == "http://localhost:4200");
    CHECK(config.timeline.cors_origins[1] == "http://localhost:3000");
}

TEST_CASE("ConfigManager singleton get() works after load()", "[config]") {
    TempConfig cfg(R"(
cameras:
  test:
    name: "Test Camera"
)");

    yolo::ConfigManager::load(cfg.path);
    const auto& config = yolo::ConfigManager::get();

    CHECK(config.cameras.count("test") == 1);
    CHECK(config.cameras.at("test").name == "Test Camera");
}

TEST_CASE("ConfigManager throws on invalid file", "[config]") {
    REQUIRE_THROWS_AS(
        yolo::ConfigManager::load("/nonexistent/path.yaml"),
        std::runtime_error
    );
}
