// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <catch_amalgamated.hpp>

#include <shapes/bounding_box.h>
#include <shapes/bounding_box_algos.h>

#include <vector>
#include <sstream>
#include <string>

namespace shapes
{

TEST_CASE("ensure_min_extent", "[range]")
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

}
