// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/path.h>

#include <vector>

namespace svg {

template <typename F>
struct Paths
{
    std::vector<shapes::PointPath2d<F>>        point_paths;
    std::vector<shapes::CubicBezierPath2d<F>>  cubic_bezier_paths;
};

} // namespace svg
