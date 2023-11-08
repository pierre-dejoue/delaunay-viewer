#pragma once

#include <graphs/graph.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>


namespace graphs
{

// Borders of a 2-manifold (which can have several components)
template <typename I>
struct BordersAndInnerEdges
{
    EdgeSoup<I> borders;
    std::size_t nb_inner_edges;
};

// Extract the borders of a 2-manifold triangulation
template <typename I>
BordersAndInnerEdges<I> extract_borders(const TriangleSoup<I>& triangles);


//
//
// Implementations
//
//


template <typename I>
BordersAndInnerEdges<I> extract_borders(const TriangleSoup<I>& triangles)
{
    using counter_type = std::map<graphs::Edge<I>, std::uint8_t>;
    using counter_value = typename counter_type::value_type;
    counter_type edge_count;
    for (const auto& triangle : triangles)
    {
        edge_count[graphs::ordered_edge(graphs::Edge<I>(triangle[0], triangle[1]))]++;
        edge_count[graphs::ordered_edge(graphs::Edge<I>(triangle[1], triangle[2]))]++;
        edge_count[graphs::ordered_edge(graphs::Edge<I>(triangle[2], triangle[0]))]++;
    }
    assert(std::all_of(std::cbegin(edge_count), std::cend(edge_count), [](const auto& kvp) { return kvp.second == 1 || kvp.second == 2; })); // 2-manifoldness

    // Extract border edges (edges whose cardinality is 1)
    auto result = [&edge_count]() -> BordersAndInnerEdges<I> {
        BordersAndInnerEdges<I> result;
        std::vector<counter_value> border_kvps;
        std::copy_if(std::cbegin(edge_count), std::cend(edge_count), std::back_inserter(border_kvps), [](const auto& kvp) { return kvp.second == 1; });
        result.borders.reserve(border_kvps.size());
        std::transform(std::begin(border_kvps), std::end(border_kvps), std::back_inserter(result.borders), [](const auto& kvp) { return std::move(kvp.first); });
        return result;
    }();
    result.nb_inner_edges = edge_count.size() - result.borders.size();

    return result;
}

} // namespace graphs
