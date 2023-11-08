// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/path.h>
#include <shapes/point_cloud.h>


namespace shapes
{

static constexpr bool CLOSED_PATH = true;

template <typename P>
PointCloud<P> to_point_cloud(PointPath<P>&& pp)
{
    PointCloud<P> pc;
    pc.vertices = std::move(pp.vertices);
    return pc;
}

template <typename P>
PointPath<P> to_point_path(PointCloud<P>&& pc, bool closed = true)
{
    PointPath<P> pp;
    pp.vertices = std::move(pc.vertices);
    pp.closed = closed;
    return pp;
}

} // namespace shapes