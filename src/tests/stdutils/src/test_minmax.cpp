// Copyright (c) 2026 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/minmax.h>

#include <cstdint>

TEST_CASE("stdutils::clamp", "[minmax]")
{
    static_assert(stdutils::clamp<int>(    -150, INT8_MIN, INT8_MAX) == INT8_MIN);
    static_assert(stdutils::clamp<int>(INT8_MIN, INT8_MIN, INT8_MAX) == INT8_MIN);
    static_assert(stdutils::clamp<int>(      42, INT8_MIN, INT8_MAX) == 42);
    static_assert(stdutils::clamp<int>(INT8_MAX, INT8_MIN, INT8_MAX) == INT8_MAX);
    static_assert(stdutils::clamp<int>(     150, INT8_MIN, INT8_MAX) == INT8_MAX);

    static_assert(stdutils::clamp<int>(    -150, INT8_MIN, INT8_MAX, std::less<int>()) == INT8_MIN);
    static_assert(stdutils::clamp<int>(INT8_MIN, INT8_MIN, INT8_MAX, std::less<int>()) == INT8_MIN);
    static_assert(stdutils::clamp<int>(      42, INT8_MIN, INT8_MAX, std::less<int>()) == 42);
    static_assert(stdutils::clamp<int>(INT8_MAX, INT8_MIN, INT8_MAX, std::less<int>()) == INT8_MAX);
    static_assert(stdutils::clamp<int>(     150, INT8_MIN, INT8_MAX, std::less<int>()) == INT8_MAX);

    static_assert(stdutils::clamp<int>(    -150, INT8_MAX, INT8_MIN, std::greater<int>()) == INT8_MIN);
    static_assert(stdutils::clamp<int>(INT8_MIN, INT8_MAX, INT8_MIN, std::greater<int>()) == INT8_MIN);
    static_assert(stdutils::clamp<int>(      42, INT8_MAX, INT8_MIN, std::greater<int>()) == 42);
    static_assert(stdutils::clamp<int>(INT8_MAX, INT8_MAX, INT8_MIN, std::greater<int>()) == INT8_MAX);
    static_assert(stdutils::clamp<int>(     150, INT8_MAX, INT8_MIN, std::greater<int>()) == INT8_MAX);

    CHECK(stdutils::clamp<int>(42, INT8_MIN, INT8_MAX) == 42);
}

TEST_CASE("stdutils::clamp with bool& clamped", "[minmax]")
{
    bool clamped = false;
    int r = 0;
    {
        r = stdutils::clamp<int>(    -150, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == INT8_MIN);
        CHECK(clamped);
        r = stdutils::clamp<int>(INT8_MIN, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == INT8_MIN);
        CHECK(!clamped);
        r = stdutils::clamp<int>(      42, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == 42);
        CHECK(!clamped);
        r = stdutils::clamp<int>(INT8_MAX, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == INT8_MAX);
        CHECK(!clamped);
        r = stdutils::clamp<int>(     150, INT8_MIN, INT8_MAX, clamped);
        CHECK(r == INT8_MAX);
        CHECK(clamped);
    }
    {
        r = stdutils::clamp<int>(    -150, INT8_MIN, INT8_MAX, clamped, std::less<int>());
        CHECK(r == INT8_MIN);
        CHECK(clamped);
        r = stdutils::clamp<int>(INT8_MIN, INT8_MIN, INT8_MAX, clamped, std::less<int>());
        CHECK(r == INT8_MIN);
        CHECK(!clamped);
        r = stdutils::clamp<int>(      42, INT8_MIN, INT8_MAX, clamped, std::less<int>());
        CHECK(r == 42);
        CHECK(!clamped);
        r = stdutils::clamp<int>(INT8_MAX, INT8_MIN, INT8_MAX, clamped, std::less<int>());
        CHECK(r == INT8_MAX);
        CHECK(!clamped);
        r = stdutils::clamp<int>(     150, INT8_MIN, INT8_MAX, clamped, std::less<int>());
        CHECK(r == INT8_MAX);
        CHECK(clamped);
    }
    {
        r = stdutils::clamp<int>(    -150, INT8_MAX, INT8_MIN, clamped, std::greater<int>());
        CHECK(r == INT8_MIN);
        CHECK(clamped);
        r = stdutils::clamp<int>(INT8_MIN, INT8_MIN, INT8_MIN, clamped, std::greater<int>());
        CHECK(r == INT8_MIN);
        CHECK(!clamped);
        r = stdutils::clamp<int>(      42, INT8_MAX, INT8_MIN, clamped, std::greater<int>());
        CHECK(r == 42);
        CHECK(!clamped);
        r = stdutils::clamp<int>(INT8_MAX, INT8_MAX, INT8_MIN, clamped, std::greater<int>());
        CHECK(r == INT8_MAX);
        CHECK(!clamped);
        r = stdutils::clamp<int>(     150, INT8_MAX, INT8_MIN, clamped, std::greater<int>());
        CHECK(r == INT8_MAX);
        CHECK(clamped);
        //r = stdutils::clamp<int>(      42, INT8_MIN, INT8_MAX, clamped, std::greater<int>());    // Would assert
    }
}
