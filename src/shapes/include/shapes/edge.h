#pragma once

#include <graphs/index.h>
#include <graphs/graph.h>
#include <graphs/graph_algos.h>
#include <shapes/point.h>

#include <algorithm>
#include <cstdint>
#include <vector>


namespace shapes
{


/**
 * Edges
 *
 * A Edge soup
 */
template <typename P, typename I = std::uint32_t>
struct Edges
{
    static_assert(graphs::IndexTraits<I>::is_valid());
    static constexpr int dim = P::dim;
    using scalar = typename P::scalar;
    using index = I;
    std::vector<P> vertices;
    graphs::EdgeSoup<I> indices;
};

template <typename F, typename I = std::uint32_t>
using Edges2d = Edges<Point2d<F>, I>;

template <typename F, typename I = std::uint32_t>
using Edges3d = Edges<Point3d<F>, I>;

template <typename P, typename I>
bool is_valid(const Edges<P, I>& edges)
{
    const I nb_vertices = static_cast<I>(edges.vertices.size());
    const bool all_valid = std::all_of(std::cbegin(edges.indices), std::cend(edges.indices), [nb_vertices](const graphs::Edge<I>& e) {
        return graphs::is_valid(e) && e.first < nb_vertices && e.second < nb_vertices;
    });
    return all_valid && !graphs::has_duplicates(edges.indices);
}

template <typename P, typename I>
std::size_t nb_edges(const Edges<P, I>& edges)
{
    assert(is_valid(edges));
    return graphs::nb_edges(edges.indices);
}

} // namespace shapes
