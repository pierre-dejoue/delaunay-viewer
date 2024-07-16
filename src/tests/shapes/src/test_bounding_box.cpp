// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <shapes/bounding_box.h>
#include <shapes/bounding_box_algos.h>

#include <cstdlib>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace shapes {

TEST_CASE("ensure_min_extent", "[bounding_box]")
{
    auto r1 = Range<float>().add(1.0);
    ensure_min_extent(r1);
    CHECK(r1.length() > 0.f);

    auto r2 = Range<float>().add(123456.0);
    ensure_min_extent(r2);
    CHECK(r2.length() > 0.f);

    auto r3 = Range<float>().add(-73321.0);
    ensure_min_extent(r3);
    CHECK(r3.length() > 0.f);
}

TEST_CASE("Bounding box with integer type", "[bounding_box]")
{
    using int_t = std::int32_t;
    BoundingBox2d<int_t> bb_int;
    bb_int.add(-1, -2).add(3, 4);
    const Vect2d<int_t> extent = bb_int.extent();
    CHECK(extent.x == 4);
    CHECK(extent.y == 6);
}

TEST_CASE("Conservative rounding to a bounding box with integer type", "[bounding_box]")
{
    using F = double;
    using int_t = std::int32_t;

    const double tol = 1E-06;

    BoundingBox2d<F> bb_f;
    bb_f.add(-1.1, -2.1).add(3.1, 4.1);
    const Vect2d<F> extent_f = bb_f.extent();
    CHECK_THAT(extent_f.x, Catch::Matchers::WithinRel(4.2, tol));
    CHECK_THAT(extent_f.y, Catch::Matchers::WithinRel(6.2, tol));

    const auto bb_i = conservative_rounding<F, int_t>(bb_f);
    static_assert(std::is_same_v<decltype(bb_i)::scalar, int_t>);
    const Vect2d<int_t> extent_i = bb_i.extent();
    CHECK(extent_i.x == 6);
    CHECK(extent_i.y == 8);
}

TEST_CASE("Conservative rounding to a bounding box with a floating-point type", "[bounding_box]")
{
    using F0 = double;      // From
    using F1 = float;       // To

    BoundingBox2d<F0> bb_in;
    bb_in.add(-5.1, -4.1).add(3.1, 2.1);

    const auto bb_out = conservative_rounding<F0, F1>(bb_in);
    static_assert(std::is_same_v<decltype(bb_out)::scalar, F1>);
    const Vect2d<F1> extent_out = bb_out.extent();
    CHECK(extent_out.x == F1{10});
    CHECK(extent_out.y == F1{8});
}

} // namespace shapes
