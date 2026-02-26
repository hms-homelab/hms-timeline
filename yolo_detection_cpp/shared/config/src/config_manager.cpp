#include "config_manager.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <fstream>

namespace yolo {

AppConfig ConfigManager::load(const std::string& config_path) {
    spdlog::info("Loading configuration from: {}", config_path);

    YAML::Node root;
    try {
        root = YAML::LoadFile(config_path);
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("Failed to load config file: " + std::string(e.what()));
    }

    config_ = parse(root);
    loaded_ = true;
    return config_;
}

const AppConfig& ConfigManager::get() {
    if (!loaded_) {
        throw std::runtime_error("ConfigManager::get() called before load()");
    }
    return config_;
}

void ConfigManager::reload(const std::string& config_path) {
    load(config_path);
    spdlog::info("Configuration reloaded");
}

AppConfig ConfigManager::parse(const YAML::Node& root) {
    AppConfig config;

    // Parse cameras
    if (auto cameras_node = root["cameras"]) {
        for (auto it = cameras_node.begin(); it != cameras_node.end(); ++it) {
            CameraConfig cam;
            cam.id = it->first.as<std::string>();
            auto cam_node = it->second;

            cam.name = cam_node["name"].as<std::string>(cam.id);
            cam.rtsp_url = cam_node["rtsp_url"].as<std::string>("");
            cam.enabled = cam_node["enabled"].as<bool>(true);

            if (auto classes_node = cam_node["classes"]) {
                for (const auto& cls : classes_node) {
                    cam.classes.push_back(cls.as<std::string>());
                }
            }

            cam.confidence_threshold =
                cam_node["confidence_threshold"].as<double>(0.5);
            cam.immediate_notification_confidence =
                cam_node["immediate_notification_confidence"].as<double>(0.70);

            config.cameras[cam.id] = std::move(cam);
        }
    }

    // Parse buffer settings
    if (auto buf = root["buffer"]) {
        config.buffer.preroll_seconds = buf["preroll_seconds"].as<int>(5);
        config.buffer.fps = buf["fps"].as<int>(15);
        config.buffer.max_buffer_size_mb = buf["max_buffer_size_mb"].as<int>(50);
    }

    // Parse detection settings
    if (auto det = root["detection"]) {
        config.detection.model_path = det["model_path"].as<std::string>("yolo11s.pt");
        config.detection.confidence_threshold = det["confidence_threshold"].as<double>(0.5);
        config.detection.immediate_notification_confidence =
            det["immediate_notification_confidence"].as<double>(0.70);
        config.detection.iou_threshold = det["iou_threshold"].as<double>(0.45);
        config.detection.max_detections = det["max_detections"].as<int>(10);

        if (auto classes_node = det["classes"]) {
            for (const auto& cls : classes_node) {
                config.detection.classes.push_back(cls.as<std::string>());
            }
        }
    }

    // Parse MQTT settings
    if (auto mqtt = root["mqtt"]) {
        config.mqtt.broker = mqtt["broker"].as<std::string>("192.168.2.15");
        config.mqtt.port = mqtt["port"].as<int>(1883);
        config.mqtt.username = mqtt["username"].as<std::string>("");
        config.mqtt.password = mqtt["password"].as<std::string>("");
        config.mqtt.topic_prefix = mqtt["topic_prefix"].as<std::string>("yolo_detection");
        config.mqtt.qos = mqtt["qos"].as<int>(1);
    }

    // Parse API settings
    if (auto api = root["api"]) {
        config.api.host = api["host"].as<std::string>("0.0.0.0");
        config.api.port = api["port"].as<int>(8000);
        config.api.workers = api["workers"].as<int>(1);
    }

    // Parse database settings
    if (auto db = root["database"]) {
        config.database.host = db["host"].as<std::string>("192.168.2.15");
        config.database.port = db["port"].as<int>(5432);
        config.database.user = db["user"].as<std::string>("maestro");
        config.database.password = db["password"].as<std::string>("");
        config.database.database = db["database"].as<std::string>("ai_context");
        config.database.pool_size = db["pool_size"].as<int>(4);
    }

    // Parse timeline settings (new section for yolo-timeline service)
    if (auto tl = root["timeline"]) {
        config.timeline.host = tl["host"].as<std::string>("0.0.0.0");
        config.timeline.port = tl["port"].as<int>(8080);
        config.timeline.static_files_path =
            tl["static_files_path"].as<std::string>("frontend/dist/browser");
        config.timeline.events_dir =
            tl["events_dir"].as<std::string>("/mnt/ssd/events");
        config.timeline.snapshots_dir =
            tl["snapshots_dir"].as<std::string>("/mnt/ssd/snapshots");
        config.timeline.detection_service_url =
            tl["detection_service_url"].as<std::string>("http://localhost:8000");

        if (auto cors = tl["cors_origins"]) {
            config.timeline.cors_origins.clear();
            for (const auto& origin : cors) {
                config.timeline.cors_origins.push_back(origin.as<std::string>());
            }
        }
    }

    // Parse logging settings
    if (auto log = root["logging"]) {
        config.logging.level = log["level"].as<std::string>("info");
        config.logging.file = log["file"].as<std::string>("logs/yolo_api.log");
        config.logging.max_bytes = log["max_bytes"].as<size_t>(10485760);
        config.logging.backup_count = log["backup_count"].as<int>(5);
    }

    spdlog::info("Loaded {} camera(s)", config.cameras.size());
    return config;
}

} // namespace yolo
