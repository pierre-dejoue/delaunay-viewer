// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <graphs/index.h>
#include <shapes/point_cloud.h>
#include <shapes/edge.h>
#include <shapes/triangle.h>

#include <cstdint>

namespace shapes {

/**
 * A healthy soup of 0-, 1-, and 2-simplices
 */
template <typename P, typename I = std::uint32_t>
struct Soup
{
    PointCloud<P>       point_cloud;
    Edges<P, I>         edges;
    Triangles<P, I>     triangles;
};

template <typename F, typename I = std::uint32_t>
using Soup2d = Soup<Point2d<F>, I>;

template <typename F, typename I = std::uint32_t>
using Soup3d = Soup<Point3d<F>, I>;

} // namespace shapes
