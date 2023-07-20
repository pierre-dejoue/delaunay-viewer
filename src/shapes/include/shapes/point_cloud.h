#pragma once

#include <shapes/point.h>

#include <vector>


namespace shapes
{

/**
 * Point Cloud
 */
template <typename P>
struct PointCloud
{
    static constexpr int dim = P::dim;
    using scalar = typename P::scalar;
    PointCloud() : vertices() {}
    std::vector<P> vertices;
};

template <typename F>
using PointCloud2d = PointCloud<Point2d<F>>;

template <typename F>
using PointCloud3d = PointCloud<Point3d<F>>;

} // namespace shapes
