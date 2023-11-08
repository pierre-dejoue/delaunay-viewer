#pragma once

#include <graphs/graph_algos.h>
#include <graphs/triangulation.h>
#include <lin/mat.h>
#include <shapes/edge.h>
#include <shapes/path.h>
#include <shapes/triangle.h>

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

namespace shapes
{

template <typename P, typename I>
std::vector<PointPath<P>> extract_borders(const Triangles<P, I>& triangles);

template <typename P, typename I>
Edges<P, I> extract_edges(const Triangles<P, I>& triangles);

template <typename P, typename I>
PointCloud<P> circumcenters(const Triangles<P, I>& triangles);


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

template <typename P, typename I>
PointCloud<P> circumcenters(const Triangles<P, I>& triangles)
{
    PointCloud<P> result;
    using F = typename P::scalar;
    result.vertices.reserve(triangles.faces.size());
    for (const auto& face : triangles.faces)
    {
        const P& a = triangles.vertices[face[0]];
        const P& b = triangles.vertices[face[1]];
        const P& c = triangles.vertices[face[2]];
        const P ab = b - a;
        const P bc = c - b;
        const P sumab = a + b;
        const P sumbc = b + c;
        const P d(F{0.5} * dot(ab, sumab), F{0.5} * dot(bc, sumbc));
        lin::mat2<F> m{ ab.x, ab.y, bc.x, bc.y };
        F det{0};
        lin::inverse(m, &det);
        if (det != F{0})
            result.vertices.emplace_back(m[0][0] * d.x + m[0][1] * d.y, m[1][0] * d.x + m[1][1] * d.y);
    }
    return result;
}

} // namespace shapes
