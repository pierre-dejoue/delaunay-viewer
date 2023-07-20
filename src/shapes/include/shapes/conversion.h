#pragma once

#include <shapes/path.h>
#include <shapes/point_cloud.h>


namespace shapes
{

template <typename P>
PointCloud<P> to_point_cloud(PointPath<F>&& pp)
{
    PointCloud<P> pc;
    pc.vertices = std::move(pp.vertices);
    return pc;
}

template <typename P>
PointPath<P> to_point_path(PointCloud<F>&& pc, bool closed = true)
{
    PointPath<P> pp;
    pp.vertices = std::move(pc.vertices);
    pp.closed = closed;
    return pp;
}

} // namespace shapes