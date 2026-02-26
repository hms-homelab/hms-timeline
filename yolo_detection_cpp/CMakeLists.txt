cmake_minimum_required(VERSION 3.20)
project(yolo_detection_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Common dependencies ──────────────────────────────────────────────────────

option(BUILD_TESTS "Build unit tests" ON)

find_package(spdlog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)          # target: yaml-cpp::yaml-cpp
find_package(nlohmann_json CONFIG REQUIRED)     # target: nlohmann_json::nlohmann_json
if(BUILD_TESTS)
  find_package(Catch2 3 CONFIG REQUIRED)        # target: Catch2::Catch2WithMain
endif()
find_package(Drogon CONFIG REQUIRED)            # target: Drogon::Drogon

# libpqxx ships only a pkg-config file on Debian — no cmake config
find_package(PkgConfig REQUIRED)
pkg_check_modules(pqxx REQUIRED IMPORTED_TARGET libpqxx)
# Creates: PkgConfig::pqxx

# ── Subdirectories ───────────────────────────────────────────────────────────

add_subdirectory(shared)
add_subdirectory(services/timeline)

# Future: add_subdirectory(services/detection)
