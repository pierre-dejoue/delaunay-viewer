// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/point.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

namespace shapes {

/**
 * Point Path
 *
 * aka polyline
 */
template <typename P>
struct PointPath
{
    static constexpr int dim = P::dim;
    using scalar = typename P::scalar;
    PointPath() : closed(false), vertices() {}
    bool closed;
    std::vector<P> vertices;
};

template <typename F>
using PointPath2d = PointPath<Point2d<F>>;

template <typename F>
using PointPath3d = PointPath<Point3d<F>>;

template <typename P>
bool valid_size(const PointPath<P>& pp)
{
    return !pp.closed || pp.vertices.size() > 2;
}

template <typename P>
bool is_valid(const PointPath<P>& pp)
{
    return valid_size(pp) && std::all_of(pp.vertices.begin(), pp.vertices.end(), [](const auto& p) { return shapes::isfinite(p); });
}

template <typename P>
std::size_t nb_edges(const PointPath<P>& pp)
{
    assert(valid_size(pp));
    const auto sz = pp.vertices.size();
    return pp.closed ? (sz > 2 ? sz : 0) : (sz > 0 ? sz - 1 : 0);
}


/**
 * Cubic Bezier Path
 *
 * Continuous curve made of cubic bezier segments.
 * The vertices whose index % 3 = 0 are the endpoints
 * The vertices whose index % 3 = 1 or 2 are the control points
 */
template <typename P>
struct CubicBezierPath
{
    static constexpr int dim = P::dim;
    using scalar = typename P::scalar;
    CubicBezierPath() : closed(true), vertices() {}
    bool closed;
    std::vector<P> vertices;
};

template <typename F>
using CubicBezierPath2d = CubicBezierPath<Point2d<F>>;

template <typename F>
using CubicBezierPath3d = CubicBezierPath<Point3d<F>>;

template <typename P>
bool valid_size(const CubicBezierPath<P>& cbp)
{
    return (cbp.vertices.size() % 3) == (cbp.closed ? 0u : 1u);
}

template <typename P>
bool is_valid(const CubicBezierPath<P>& cbp)
{
    return valid_size(cbp) && std::all_of(cbp.vertices.begin(), cbp.vertices.end(), [](const auto& p) { return shapes::isfinite(p); });
}

template <typename P>
std::size_t nb_edges(const CubicBezierPath<P>& cbp)
{
    assert(valid_size(cbp));
    return cbp.vertices.size() / 3;
}

} // namespace shapes
