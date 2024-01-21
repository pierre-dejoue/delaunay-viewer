// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <dt/dt_impl.h>
#include <shapes/point_cloud.h>
#include <shapes/proximity_graphs.h>
#include <shapes/triangle_algos.h>
#include <stdutils/io.h>
#include <stdutils/macros.h>

namespace delaunay {

/**
 * See graphs/proximity.h for more information regarding the proximity graphs
 */

template <typename P, typename I = std::uint32_t>
shapes::Edges<P, I> nearest_neighbor(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler);

template <typename P, typename I = std::uint32_t>
shapes::Edges<P, I> minimum_spanning_tree(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler);

template <typename P, typename I = std::uint32_t>
shapes::Edges<P, I> relative_neighborhood_graph(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler);

template <typename P, typename I = std::uint32_t>
shapes::Edges<P, I> gabriel_graph(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler);

template <typename P, typename I = std::uint32_t>
shapes::Edges<P, I> delaunay_triangulation(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler);


//
//
// Implementation
//
//


namespace details {

template <typename P, typename I, typename Func>
shapes::Edges<P, I> generic_proximity_graph(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler, Func func)
{
    using F = typename P::scalar;

    // Delaunay triangulation
    auto [delaunay_name, delaunay_algo] = delaunay::get_ref_impl<F, I>(&err_handler);
    UNUSED(delaunay_name);
    if (!delaunay_algo)
    {
        err_handler(stdutils::io::Severity::ERR, "Could not find a Delaunay triangulation algo");
        return shapes::Edges<P>();
    }
    delaunay_algo->add_steiner(pc);
    const shapes::Triangles2d<F, I> triangles = delaunay_algo->triangulate(delaunay::TriangulationPolicy::PointCloud);

    // Compute proximity graph
    return func(triangles);
}

}

template <typename P, typename I>
shapes::Edges<P, I> nearest_neighbor(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler)
{
    return details::generic_proximity_graph<P, I>(pc, err_handler, &shapes::nearest_neighbor<P, I>);
}

template <typename P, typename I>
shapes::Edges<P, I> minimum_spanning_tree(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler)
{
    return details::generic_proximity_graph<P, I>(pc, err_handler, &shapes::minimum_spanning_tree<P, I>);
}

template <typename P, typename I>
shapes::Edges<P, I> relative_neighborhood_graph(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler)
{
    return details::generic_proximity_graph<P, I>(pc, err_handler, &shapes::relative_neighborhood_graph<P, I>);
}

template <typename P, typename I>
shapes::Edges<P, I> gabriel_graph(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler)
{
    return details::generic_proximity_graph<P, I>(pc, err_handler, &shapes::gabriel_graph<P, I>);
}

template <typename P, typename I>
shapes::Edges<P, I> delaunay_triangulation(const shapes::PointCloud<P>& pc, const stdutils::io::ErrorHandler& err_handler)
{
    return details::generic_proximity_graph<P, I>(pc, err_handler, &shapes::extract_edges<P, I>);
}

} // namespace delaunay
