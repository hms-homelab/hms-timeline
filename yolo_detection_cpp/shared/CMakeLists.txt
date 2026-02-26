# Shared library used by both yolo_detection and yolo_timeline

add_library(yolo_shared STATIC
    config/src/config_manager.cpp
    common/src/time_utils.cpp
    db/src/db_pool.cpp
    db/src/api_queries.cpp
)

target_include_directories(yolo_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/config/include
    ${CMAKE_CURRENT_SOURCE_DIR}/common/include
    ${CMAKE_CURRENT_SOURCE_DIR}/db/include
)

target_link_libraries(yolo_shared PUBLIC
    spdlog::spdlog
    yaml-cpp::yaml-cpp
    nlohmann_json::nlohmann_json
    PkgConfig::pqxx
)

if(BUILD_TESTS)
    include(CTest)
    include(Catch)

    add_executable(shared_tests
        config/tests/config_manager_test.cpp
        common/tests/time_utils_test.cpp
        db/tests/api_queries_test.cpp
    )

    target_link_libraries(shared_tests PRIVATE
        yolo_shared
        Catch2::Catch2WithMain
    )

    catch_discover_tests(shared_tests)
endif()
