#pragma once

#include <graphs/graph_algos.h>
#include <graphs/proximity.h>
#include <shapes/edge.h>
#include <shapes/triangle.h>

#include <cassert>
#include <cstdint>
#include <vector>

namespace shapes
{

/**
 * See graphs/proximity.h for more information regarding the proximity graphs
 */

template <typename P, typename I = std::uint32_t>
Edges<P, I> nearest_neighbor(const Triangles<P, I>& triangles);

template <typename P, typename I = std::uint32_t>
Edges<P, I> minimum_spanning_tree(const Triangles<P, I>& triangles);

template <typename P, typename I = std::uint32_t>
Edges<P, I> relative_neighborhood_graph(const Triangles<P, I>& triangles);

template <typename P, typename I = std::uint32_t>
Edges<P, I> gabriel_graph(const Triangles<P, I>& triangles);

//
//
// Implementations
//
//


namespace details
{

template <typename F, typename I>
struct WeightEdge
{
    using index = I;

    WeightEdge() : m_edge{0, 0}, m_length{0} {}

    const graphs::Edge<I> edge() const { return m_edge; }
    F weight() const { return m_length; }

    graphs::Edge<I> m_edge;
    F m_length;
};

template <typename F, typename I>
using WeightEdges = std::vector<WeightEdge<F, I>>;

template <typename P, typename I, typename Func>
Edges<P, I> generic_proximity_graph(const Triangles<P, I>& triangles, Func func)
{
    using F = typename P::scalar;

    // Extract the edges from the triangulation
    WeightEdges<F, I> proxi_edges = [&triangles]() {
        WeightEdges<F, I> result;
        auto edge_soup = graphs::to_edge_soup<I>(triangles.faces);
        result.reserve(edge_soup.size());
        std::for_each(std::cbegin(edge_soup), std::cend(edge_soup), [&triangles, &result](const auto& e) {
            auto& w_edge = result.emplace_back();
            w_edge.m_edge = e;
            w_edge.m_length = shapes::norm(triangles.vertices[e.second] - triangles.vertices[e.first]);
        });
        return result;
    }();

    // Compute the proximity graph
    const auto graph_end = func(proxi_edges.begin(), proxi_edges.end());

    // Return edge soup
    Edges<P, I> result;
    result.indices.reserve(static_cast<std::size_t>(std::distance(proxi_edges.begin(), graph_end)));
    std::transform(proxi_edges.begin(), graph_end, std::back_inserter(result.indices), [](const auto& proxi_edge) { return proxi_edge.edge(); });
    result.vertices = triangles.vertices;
    return result;
}

}

template <typename P, typename I>
Edges<P, I> nearest_neighbor(const Triangles<P, I>& triangles)
{
    using F = typename P::scalar;
    return details::generic_proximity_graph<P, I>(triangles, &graphs::nearest_neighbor<typename details::WeightEdges<F, I>::iterator>);
}

template <typename P, typename I>
Edges<P, I> minimum_spanning_tree(const Triangles<P, I>& triangles)
{
    using F = typename P::scalar;
    return details::generic_proximity_graph<P, I>(triangles, &graphs::minimum_spanning_tree<typename details::WeightEdges<F, I>::iterator>);
}

template <typename P, typename I>
Edges<P, I> relative_neighborhood_graph(const Triangles<P, I>& triangles)
{
    using F = typename P::scalar;
    using WeightEdgeIt = typename details::WeightEdges<F, I>::iterator;
    const auto& vertices = triangles.vertices;
    const auto rng_gen = [&vertices](WeightEdgeIt begin, WeightEdgeIt end) {
        return graphs::relative_neighborhood_graph(begin, end, [&vertices](const I p, const I q) { return shapes::norm(vertices[q] - vertices[p]); });
    };
    return details::generic_proximity_graph<P, I>(triangles, rng_gen);
}

template <typename P, typename I>
Edges<P, I> gabriel_graph(const Triangles<P, I>& triangles)
{
    using F = typename P::scalar;
    using WeightEdgeIt = typename details::WeightEdges<F, I>::iterator;
    const auto& vertices = triangles.vertices;
    const auto gg_gen = [&vertices](WeightEdgeIt begin, WeightEdgeIt end) {
        return graphs::gabriel_graph(begin, end, [&vertices](const I p, const I q) { return shapes::norm(vertices[q] - vertices[p]); });
    };
    return details::generic_proximity_graph<P, I>(triangles, gg_gen);
}


} // namespace shapes
