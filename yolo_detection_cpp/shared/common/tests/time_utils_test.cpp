#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "time_utils.h"

using namespace yolo::time_utils;

TEST_CASE("ISO 8601 round-trip", "[time]") {
    auto now = std::chrono::system_clock::now();
    auto str = to_iso8601(now);
    auto parsed = from_iso8601(str);

    REQUIRE(parsed.has_value());
    // Should be within 1 microsecond
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(
        now - *parsed
    ).count();
    CHECK(std::abs(diff) <= 1);
}

TEST_CASE("Parse ISO 8601 datetime", "[time]") {
    auto tp = from_iso8601("2026-02-25T10:30:00");
    REQUIRE(tp.has_value());

    auto str = to_iso8601(*tp);
    CHECK(str == "2026-02-25T10:30:00");
}

TEST_CASE("Parse ISO 8601 datetime with microseconds", "[time]") {
    auto tp = from_iso8601("2026-02-25T10:30:00.123456");
    REQUIRE(tp.has_value());

    auto str = to_iso8601(*tp);
    CHECK(str == "2026-02-25T10:30:00.123456");
}

TEST_CASE("Parse date-only string", "[time]") {
    auto tp = from_date_string("2026-02-25");
    REQUIRE(tp.has_value());

    auto str = to_iso8601(*tp);
    CHECK(str == "2026-02-25T00:00:00");
}

TEST_CASE("Start and end of day", "[time]") {
    auto tp = from_iso8601("2026-02-25T14:30:45.123");
    REQUIRE(tp.has_value());

    auto sod = start_of_day(*tp);
    auto eod = end_of_day(*tp);

    CHECK(to_iso8601(sod) == "2026-02-25T00:00:00");
    CHECK(to_iso8601(eod) == "2026-02-26T00:00:00");
}

TEST_CASE("to_date_string", "[time]") {
    auto tp = from_iso8601("2026-12-31T23:59:59");
    REQUIRE(tp.has_value());

    CHECK(to_date_string(*tp) == "2026-12-31");
}

TEST_CASE("PostgreSQL timestamp to ISO 8601", "[time]") {
    CHECK(pg_timestamp_to_iso8601("2026-02-25 10:30:00.123456") ==
          "2026-02-25T10:30:00.123456");
    CHECK(pg_timestamp_to_iso8601("2026-02-25 10:30:00") ==
          "2026-02-25T10:30:00");
    CHECK(pg_timestamp_to_iso8601("") == "");
}

TEST_CASE("Invalid input returns nullopt", "[time]") {
    CHECK_FALSE(from_iso8601("").has_value());
    CHECK_FALSE(from_iso8601("not-a-date").has_value());
    CHECK_FALSE(from_date_string("").has_value());
    CHECK_FALSE(from_date_string("invalid").has_value());
}

TEST_CASE("now_iso8601 returns non-empty string", "[time]") {
    auto str = now_iso8601();
    CHECK_FALSE(str.empty());
    // Should start with a year
    CHECK(str.substr(0, 2) == "20");
    // Should contain T separator
    CHECK(str.find('T') != std::string::npos);
}
