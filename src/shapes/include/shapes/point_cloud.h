// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/point.h>

#include <algorithm>
#include <vector>

namespace shapes {

/**
 * Point Cloud
 */
template <typename P>
struct PointCloud
{
    static constexpr int dim = P::dim;
    using scalar = typename P::scalar;
    std::vector<P> vertices;
};

template <typename F>
using PointCloud2d = PointCloud<Point2d<F>>;

template <typename F>
using PointCloud3d = PointCloud<Point3d<F>>;

template <typename P>
bool is_valid(const PointCloud<P>& pc)
{
    return std::all_of(pc.vertices.begin(), pc.vertices.end(), [](const auto& p) { return shapes::isfinite(p); });
}

} // namespace shapes
