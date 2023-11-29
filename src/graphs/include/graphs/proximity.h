// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <graphs/index.h>
#include <graphs/union_find.h>
#include <stdutils/algorithm.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <set>
#include <vector>

namespace graphs
{

/**
 * The algorithms in this file generate proximity graphs.
 *
 * They take as input a list of weighted edges and output a sublist of those edges that constitute the said proximity graph.
 * E.g. the closest neighbors edges for the nearest neightbor (NN) graph.
 *
 * More precisely the interface of those algorithms works similarly to std::partition():
 *  - The input is a range defined by two forward iterators, which value type must be a weighted edge (see below)
 *  - The range is reordered so that the output edges are at the beginning, and the end iterator of that range is returned by the algorithm
 *  - Some of the algorithms also require a weight function that returns the weight for any edge (not necessarily on the input list of edges)
 *
 * A weighted edge is any class with the following members:
 * - index is a typename corresponding to the type of the vertex index
 * - const graph::Edge<index>& edge() const
 * - F weight() const
 *
 *
 * References:
 *  - J.S.B. Mitchell and W. Mulzer "Proximity Algorithms." Chap. 32 in: Handbook of Discrete and Computational Geometry, 3rd edition.
 *    https://www.csun.edu/~ctoth/Handbook/HDCG3.html
 */

// NN
template <typename WeightedEdgeIt>
WeightedEdgeIt nearest_neighbor(WeightedEdgeIt begin, WeightedEdgeIt end);

// MST
template <typename WeightedEdgeIt>
WeightedEdgeIt minimum_spanning_tree(WeightedEdgeIt begin, WeightedEdgeIt end);

// RNG
template <typename WeightedEdgeIt, typename WeightFunc>
WeightedEdgeIt relative_neighborhood_graph(WeightedEdgeIt begin, WeightedEdgeIt end, WeightFunc weight);

// GG
template <typename WeightedEdgeIt, typename WeightFunc>
WeightedEdgeIt gabriel_graph(WeightedEdgeIt begin, WeightedEdgeIt end, WeightFunc weight);


//
//
// Implementations
//
//


template <typename WeightedEdgeIt>
WeightedEdgeIt nearest_neighbor(WeightedEdgeIt begin, WeightedEdgeIt end)
{
    using I = typename std::iterator_traits<WeightedEdgeIt>::value_type::index;

    // Sort edges by weight
    std::sort(begin, end, [](const auto& lhs, const auto& rhs) { return lhs.weight() < rhs.weight(); });

    // Support vector containing the degree of each vertex
    I max_index = 0;
    std::for_each(begin, end, [&max_index](const auto& edge) {
        stdutils::max_update(max_index, edge.edge().orig());
        stdutils::max_update(max_index, edge.edge().dest());
    });
    assert(max_index <= IndexTraits<I>::max_valid_index());
    std::vector<std::uint8_t> vertex_degree(max_index + 1, 0u);

    // Build the nearest-neighbor graph
    WeightedEdgeIt nn_end = begin;
    WeightedEdgeIt current = begin;
    std::size_t point_count = 0;
    while (current != end && point_count < vertex_degree.size())
    {
        const I i = current->edge().orig();
        const I j = current->edge().dest();
        auto& deg_i = vertex_degree[i];
        auto& deg_j = vertex_degree[j];
        bool is_nn_edge = false;
        if (deg_i == 0)
        {
            is_nn_edge = true;
            point_count++;
        }
        if (deg_j == 0)
        {
            is_nn_edge = true;
            point_count++;
        }
        if (is_nn_edge)
        {
            deg_i++;
            deg_j++;
            std::swap(*nn_end, *current);
            nn_end++;
        }
        current++;
    }

    return nn_end;
}

// Compute the MST with Kruskal's algorithm
template <typename WeightedEdgeIt>
WeightedEdgeIt minimum_spanning_tree(WeightedEdgeIt begin, WeightedEdgeIt end)
{
    using I = typename std::iterator_traits<WeightedEdgeIt>::value_type::index;

    // Sort edges by weight
    std::sort(begin, end, [](const auto& lhs, const auto& rhs) { return lhs.weight() < rhs.weight(); });

    // Union-find structure to identify components
    I max_index = 0;
    std::for_each(begin, end, [&max_index](const auto& edge) {
        stdutils::max_update(max_index, edge.edge().orig());
        stdutils::max_update(max_index, edge.edge().dest());
    });
    assert(max_index <= IndexTraits<I>::max_valid_index());
    std::vector<std::uint8_t> vertex_degree(max_index + 1, 0u);
    UnionFind<I> components(max_index + 1);

    // Build the minimum spanning tree
    WeightedEdgeIt mst_end = begin;
    WeightedEdgeIt current = begin;
    while (current != end)
    {
        const I i = current->edge().orig();
        const I j = current->edge().dest();
        const I comp_i = components.find(i);
        const I comp_j = components.find(j);
        if (comp_i != comp_j)
        {
            // Merge sub-trees
            components.subset_union(comp_i, comp_j);
            std::swap(*mst_end, *current);
            mst_end++;
        }
        current++;
    }

    return mst_end;
}

// Naive O(n^2) implementation of the RNG
template <typename WeightedEdgeIt, typename WeightFunc>
WeightedEdgeIt relative_neighborhood_graph(WeightedEdgeIt begin, WeightedEdgeIt end, WeightFunc weight)
{
    using I = typename std::iterator_traits<WeightedEdgeIt>::value_type::index;

    std::set<I> vertices;
    std::for_each(begin, end, [&vertices](const auto& edge) {
        const I i = edge.edge().orig();
        const I j = edge.edge().dest();
        vertices.insert(i);
        vertices.insert(j);
    });

    WeightedEdgeIt rng_end = begin;
    WeightedEdgeIt current = begin;
    while (current != end)
    {
        const I i = current->edge().orig();
        const I j = current->edge().dest();
        const auto w_ij = current->weight();
        const bool exclusion_zone_is_empty = std::none_of(vertices.cbegin(), vertices.cend(), [i, j, w_ij, &weight](const I k) {
            return weight(i, k) < w_ij && weight(j, k) < w_ij;
        });
        if (exclusion_zone_is_empty)
        {
            // The edge belongs to the RNG
            std::swap(*rng_end, *current);
            rng_end++;
        }
        current++;
    }

    return rng_end;
}

// Naive O(n^2) implementation of the GG
template <typename WeightedEdgeIt, typename WeightFunc>
WeightedEdgeIt gabriel_graph(WeightedEdgeIt begin, WeightedEdgeIt end, WeightFunc weight)
{
    using I = typename std::iterator_traits<WeightedEdgeIt>::value_type::index;

    std::set<I> vertices;
    std::for_each(begin, end, [&vertices](const auto& edge) {
        const I i = edge.edge().orig();
        const I j = edge.edge().dest();
        vertices.insert(i);
        vertices.insert(j);
    });

    WeightedEdgeIt gg_end = begin;
    WeightedEdgeIt current = begin;
    while (current != end)
    {
        const I i = current->edge().orig();
        const I j = current->edge().dest();
        const auto w_ij = current->weight();
        const auto w_ij_sq = w_ij * w_ij;
        const bool exclusion_zone_is_empty = std::none_of(vertices.cbegin(), vertices.cend(), [i, j, w_ij_sq, &weight](const I k) {
            const auto w_ik = weight(i, k);
            const auto w_jk = weight(j, k);
            return (w_ik * w_ik + w_jk * w_jk) < w_ij_sq;
        });
        if (exclusion_zone_is_empty)
        {
            // The edge belongs to the GG
            std::swap(*gg_end, *current);
            gg_end++;
        }
        current++;
    }

    return gg_end;
}

} // namespace graphs
