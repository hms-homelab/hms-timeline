#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <yaml-cpp/yaml.h>

namespace yolo {

struct CameraConfig {
    std::string id;
    std::string name;
    std::string rtsp_url;
    bool enabled = true;
    std::vector<std::string> classes;
    double confidence_threshold = 0.5;
    double immediate_notification_confidence = 0.70;
};

struct BufferConfig {
    int preroll_seconds = 5;
    int fps = 15;
    int max_buffer_size_mb = 50;
};

struct DetectionConfig {
    std::string model_path = "yolo11s.pt";
    double confidence_threshold = 0.5;
    double immediate_notification_confidence = 0.70;
    double iou_threshold = 0.45;
    std::vector<std::string> classes;
    int max_detections = 10;
};

struct MqttConfig {
    std::string broker = "192.168.2.15";
    int port = 1883;
    std::string username;
    std::string password;
    std::string topic_prefix = "yolo_detection";
    int qos = 1;
};

struct ApiConfig {
    std::string host = "0.0.0.0";
    int port = 8000;
    int workers = 1;
};

struct DatabaseConfig {
    std::string host = "192.168.2.15";
    int port = 5432;
    std::string user = "maestro";
    std::string password;
    std::string database = "ai_context";
    int pool_size = 4;
};

struct TimelineConfig {
    std::string host = "0.0.0.0";
    int port = 8080;
    std::string static_files_path = "frontend/dist/browser";
    std::string events_dir = "/mnt/ssd/events";
    std::string snapshots_dir = "/mnt/ssd/snapshots";
    std::string detection_service_url = "http://localhost:8000";
    std::vector<std::string> cors_origins = {"http://localhost:4200"};
};

struct LoggingConfig {
    std::string level = "info";
    std::string file = "logs/yolo_api.log";
    size_t max_bytes = 10485760;
    int backup_count = 5;
};

struct AppConfig {
    std::unordered_map<std::string, CameraConfig> cameras;
    BufferConfig buffer;
    DetectionConfig detection;
    MqttConfig mqtt;
    ApiConfig api;
    DatabaseConfig database;
    TimelineConfig timeline;
    LoggingConfig logging;
};

class ConfigManager {
public:
    /// Load configuration from a YAML file
    static AppConfig load(const std::string& config_path);

    /// Get the singleton config (must call load() first)
    static const AppConfig& get();

    /// Reload configuration from file
    static void reload(const std::string& config_path);

private:
    static AppConfig parse(const YAML::Node& root);
    static inline AppConfig config_;
    static inline bool loaded_ = false;
};

} // namespace yolo
