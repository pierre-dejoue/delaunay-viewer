// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <graphs/graph_algos.h>
#include <shapes/edge.h>
#include <shapes/path.h>

#include <algorithm>
#include <vector>

namespace shapes {

template <typename P>
PointPath<P> extract_endpoints(const CubicBezierPath<P>& cbp);

// Extract Paths from an edge soup. See notes for function graphs::extract_paths
template <typename P, typename I>
std::vector<PointPath<P>> extract_paths(const Edges<P, I>& edges);

// Flip the 'closed' boolean of the point path. Return true iff it was actualy flipped.
template <typename P>
bool flip_open_closed(PointPath<P>& pp);


//
//
// Implementation
//
//


template <typename P>
PointPath<P> extract_endpoints(const CubicBezierPath<P>& cbp)
{
    PointPath<P> pp;
    const std::size_t segs = nb_edges(cbp);
    pp.vertices.reserve(segs + 1);
    for (unsigned int idx = 0; idx < segs; idx++)
    {
        pp.vertices.emplace_back(cbp.vertices[3 * idx]);
    }
    if (!cbp.closed)
    {
        assert(cbp.vertices.size() == 3 * segs + 1);
        pp.vertices.emplace_back(cbp.vertices.back());
    }
    pp.closed = cbp.closed;
    return pp;
}


template <typename P, typename I>
std::vector<PointPath<P>> extract_paths(const Edges<P, I>& edges)
{
    std::vector<PointPath<P>> result;
    const std::vector<graphs::Path<I>> graph_paths = graphs::extract_paths(edges.indices);
    for (const auto& graph_path : graph_paths)
    {
        PointPath<P>& pp = result.emplace_back();
        pp.closed = graph_path.closed;
        pp.vertices.reserve(graph_path.vertices.size());
        std::transform(std::cbegin(graph_path.vertices), std::cend(graph_path.vertices), std::back_inserter(pp.vertices), [&edges](const I& idx) { return edges.vertices[idx]; });
    }
    return result;
}

template <typename P>
bool flip_open_closed(PointPath<P>& pp)
{
    const bool pre_closed = pp.closed;
    pp.closed = !pre_closed && (pp.vertices.size() > 2);
    assert(is_valid(pp));
    return pp.closed != pre_closed;
}

} // namespace shapes
