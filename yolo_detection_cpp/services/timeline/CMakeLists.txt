add_executable(yolo_timeline
    src/main.cpp
    src/cors_filter.cpp
    src/controllers/ui_api_controller.cpp
    src/controllers/media_controller.cpp
)

target_include_directories(yolo_timeline PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(yolo_timeline PRIVATE
    hms_shared
    Drogon::Drogon
)

if(BUILD_TESTS)
    add_executable(timeline_tests
        tests/controllers_test.cpp
    )

    target_include_directories(timeline_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/generated
    )

    target_link_libraries(timeline_tests PRIVATE
        hms_shared
        Drogon::Drogon
        Catch2::Catch2WithMain
    )

    include(Catch)
    catch_discover_tests(timeline_tests)
endif()
