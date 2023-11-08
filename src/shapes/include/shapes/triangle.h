// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <graphs/index.h>
#include <graphs/graph.h>
#include <graphs/graph_algos.h>
#include <shapes/point.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>


namespace shapes
{

/**
 * Triangles
 *
 * A triangle soup
 */
template <typename P, typename I = std::uint32_t>
struct Triangles
{
    static_assert(graphs::IndexTraits<I>::is_valid());
    static constexpr int dim = P::dim;
    using scalar = typename P::scalar;
    using index = I;
    using face = graphs::Triangle<I>;
    std::vector<P> vertices;
    graphs::TriangleSoup<I> faces;
};

template <typename F, typename I = std::uint32_t>
using Triangles2d = Triangles<Point2d<F>, I>;

template <typename F, typename I = std::uint32_t>
using Triangles3d = Triangles<Point3d<F>, I>;

template <typename P, typename I>
bool is_valid(const Triangles<P, I>& triangles)
{
    const I nb_vertices = static_cast<I>(triangles.vertices.size());
    return std::all_of(std::cbegin(triangles.faces), std::cend(triangles.faces), [nb_vertices](const typename Triangles<P>::face& f) {
        return graphs::is_valid(f) && f[0] < nb_vertices && f[1] < nb_vertices && f[2] < nb_vertices;
    });
}

template <typename P, typename I>
std::size_t nb_edges(const Triangles<P, I>& triangles)
{
    assert(is_valid(triangles));
    return graphs::nb_edges(triangles.faces);
}

} // namespace shapes
