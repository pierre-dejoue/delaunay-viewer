// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <stdutils/range.h>

TEST_CASE("Merge ranges", "[range]")
{
    using F = float;

    const stdutils::Range<F> empty_range;
    REQUIRE(empty_range.is_populated() == false);

    stdutils::Range<F> range_a;
    REQUIRE(empty_range.is_populated() == false);
    range_a.merge(empty_range);
    CHECK(range_a.is_populated() == false);
    range_a.add(-1.0f);
    CHECK(range_a.is_populated() == true);
    range_a.merge(empty_range);
    CHECK(range_a.is_populated() == true);
    CHECK(range_a.min == -1.0f);
    CHECK(range_a.max == -1.0f);

    stdutils::Range<F> range_b(-0.5f, 0.5f);
    CHECK(range_b.is_populated() == true);
    CHECK(range_b.extent() == 1.0f);
    range_b.merge(empty_range);
    CHECK(range_b.is_populated() == true);
    CHECK(range_b.extent() == 1.0f);
    range_b.merge(range_a);
    CHECK(range_b.extent() == 1.5f);
}

TEST_CASE("Scale range around center", "[range]")
{
    using F = double;
    constexpr F tol = 1E-06;

    constexpr F zoom = 1.1;     // + 10%

    const stdutils::Range<F> range_in(1.0, 3.0);

    CHECK_THAT(range_in.length(), Catch::Matchers::WithinRel(2.0, tol));
    const auto range_out = stdutils::scale_around_center(range_in, zoom);
    CHECK_THAT(range_out.length(), Catch::Matchers::WithinRel(range_in.length() * zoom, tol));
}

TEST_CASE("Conservative rounding to a range with a floating-point type", "[range]")
{
    using F0 = double;      // From
    using F1 = float;       // To

    stdutils::Range<F0> range_in;
    REQUIRE(range_in.is_populated() == false);
    range_in.add(-5.1).add(3.1);
    REQUIRE(range_in.is_populated() == true);

    const auto range_out = stdutils::conservative_rounding<F0, F1>(range_in);
    static_assert(std::is_same_v<decltype(range_out)::scalar, F1>);
    const F1 extent_out = range_out.extent();
    CHECK(extent_out == F1{10});
}
