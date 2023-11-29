// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <graphs/graph_algos.h>
#include <graphs/triangulation.h>
#include <shapes/edge.h>
#include <shapes/path.h>
#include <shapes/triangle.h>

#include <algorithm>
#include <cassert>
#include <vector>

namespace shapes
{

template <typename P, typename I>
std::vector<PointPath<P>> extract_borders(const Triangles<P, I>& triangles);

template <typename P, typename I>
Edges<P, I> extract_edges(const Triangles<P, I>& triangles);


//
//
// Implementations
//
//


template <typename P, typename I>
std::vector<PointPath<P>> extract_borders(const Triangles<P, I>& triangles)
{
    const graphs::BordersAndInnerEdges<I> edges = graphs::extract_borders(triangles.faces);

    // EdgeSoup -> Paths
    const auto paths = graphs::extract_paths(edges.borders);
    std::vector<PointPath<P>> result;
    result.reserve(paths.size());
    for (const auto& path : paths)
    {
        auto& point_path = result.emplace_back();
        point_path.vertices.reserve(path.vertices.size());
        std::transform(std::cbegin(path.vertices), std::cend(path.vertices), std::back_inserter(point_path.vertices), [&triangles](const auto& i) { return triangles.vertices[i]; });
        point_path.closed = path.closed;
        assert(is_valid(point_path));
    }

    return result;
}

template <typename P, typename I>
Edges<P, I> extract_edges(const Triangles<P, I>& triangles)
{
    Edges<P, I> result;
    result.indices = graphs::to_edge_soup<I>(triangles.faces);
    result.vertices = triangles.vertices;
    return result;
}

} // namespace shapes
