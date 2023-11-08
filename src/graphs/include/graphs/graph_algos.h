#pragma once

#include <graphs/graph.h>
#include <stdutils/algorithm.h>
#include <stdutils/macros.h>

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <set>


namespace graphs
{

// Validate (exclude loop edges, duplicate edges, invalid triangles, improper size, etc.)
template <typename I>
bool has_duplicates(const EdgeSoup<I>& edges);
template <typename I>
bool has_duplicates(const Path<I>& path);
template <typename I>
bool is_valid(const Edge<I>& edge);
template <typename I>
bool is_valid(const EdgeSoup<I>& path);
template <typename I>
bool is_valid(const Path<I>& path);
template <typename I>
bool is_valid(const Triangle<I>& t);
template <typename I>
bool is_valid(const TriangleSoup<I>& triangles);

// Return an edge <i, j> such that i < j
template <typename I>
constexpr Edge<I> ordered_edge(const Edge<I>& edge);

// Flip triangle orientation
template <typename I>
constexpr void flip_orientation(Triangle<I>& t);

// Filter
template <typename I>
void filter_out_duplicates(EdgeSoup<I>& edges);
template <typename I>
void filter_out_loops(EdgeSoup<I>& edges);
template <typename I>
void filter_out_duplicates_and_loops(EdgeSoup<I>& edges);

// Count
template <typename I>
std::size_t nb_vertices(const EdgeSoup<I>& edges);
template <typename I>
std::size_t nb_vertices(const Path<I>& path);
template <typename I>
std::size_t nb_vertices(const TriangleSoup<I>& triangles);
template <typename I>
std::size_t nb_edges(const EdgeSoup<I>& edges);
template <typename I>
std::size_t nb_edges(const Path<I>& path);
template <typename I>
std::size_t nb_edges(const TriangleSoup<I>& triangles);

// Indices
template <typename I>
std::pair<I, I> minmax_indices(const EdgeSoup<I>& edges);
template <typename I>
std::pair<I, I> minmax_indices(const Path<I>& path);
template <typename I>
std::pair<I, I> minmax_indices(const TriangleSoup<I>& triangles);

// Reindex to interval [0; n-1]. Preserve vertex order
template <typename I>
void remap_indices(EdgeSoup<I>& edges);
template <typename I>
void remap_indices(Path<I>& path);
template <typename I>
void remap_indices(TriangleSoup<I>& triangles);

// Conversions. The otput is_valid providing the input is.
template <typename I>
VertexSet<I> to_vertex_set(const EdgeSoup<I>& edges);
template <typename I>
VertexSet<I> to_vertex_set(const Path<I>& path);
template <typename I>
VertexSet<I> to_vertex_set(const TriangleSoup<I>& triangles);
template <typename I>
EdgeSoup<I> to_edge_soup(const Path<I>& path);
template <typename I>
EdgeSoup<I> to_edge_soup(const TriangleSoup<I>& triangles);

// Degree: The degree of a vertex is the number of neighbour (i.e. edges connected to it).
template <typename I>
std::size_t min_degree(const EdgeSoup<I>& edges);
template <typename I>
std::size_t max_degree(const EdgeSoup<I>& edges);
using MinMaxDeg = std::pair<std::size_t, std::size_t>;
template <typename I>
MinMaxDeg minmax_degree(const EdgeSoup<I>& edges);

// Extract Paths from EdgeSoup
//
// Notes:
//  - All input edges are represented once in the output
//  - The input graph doesn't have to be 1-manifold, however vertices whose degree is > 2 are considered path endpoints
//
template <typename I>
std::vector<Path<I>> extract_paths(const EdgeSoup<I>& edges);


//
//
// Implementations
//
//

template <typename I>
bool has_duplicates(const EdgeSoup<I>& edges)
{
    std::set<Edge<I>> ordered_edges;
    return std::any_of(edges.cbegin(), edges.cend(), [&ordered_edges](const auto& e) { return !ordered_edges.insert(ordered_edge(e)).second; });
}

template <typename I>
bool has_duplicates(const Path<I>& path)
{
    std::set<I> unique_vertices;
    return std::any_of(path.vertices.cbegin(), path.vertices.cend(), [&unique_vertices](const auto& p) { return !unique_vertices.insert(p).second; });
}

template <typename I>
bool is_valid(const Edge<I>& edge)
{
    static_assert(IndexTraits<I>::is_valid());
    return is_defined(edge) && !is_loop(edge);
}

template <typename I>
bool is_valid(const EdgeSoup<I>& edges)
{
    static_assert(IndexTraits<I>::is_valid());
    const bool all_valid = std::all_of(std::cbegin(edges), std::cend(edges), [](const auto&e) { return is_valid(e); });
    return all_valid && !has_duplicates(edges);
}

template <typename I>
bool is_valid(const Path<I>& path)
{
    static_assert(IndexTraits<I>::is_valid());
    return !has_duplicates(path) && (!path.closed || path.vertices.size() > 2);
}

// Exclude triangles with repeated indices
template <typename I>
bool is_valid(const Triangle<I>& t)
{
    return t[0] != t[1] && t[1] != t[2] && t[2] != t[0];
}

template <typename I>
bool is_valid(const TriangleSoup<I>& triangles)
{
    static_assert(IndexTraits<I>::is_valid());
    return std::all_of(std::cbegin(triangles), std::cend(triangles), [](const auto& t) { return is_valid(t); });
}

template <typename I>
constexpr Edge<I> ordered_edge(const Edge<I>& edge)
{
    return std::minmax(edge.first, edge.second);
}

template <typename I>
constexpr void flip_orientation(Triangle<I>& t)
{
    std::swap(t.vertices[1], t.vertices[2]);
}

template <typename I>
void filter_out_duplicates(EdgeSoup<I>& edges)
{
    std::set<Edge<I>> ordered_edges;
    const auto past_kept_edges = std::remove_if(std::begin(edges), std::end(edges), [&ordered_edges](const Edge<I>& e) {
        const bool inserted = ordered_edges.insert(ordered_edge(e)).second;
        return !inserted;
    });
    edges.erase(past_kept_edges, std::end(edges));
    assert(edges.size() == ordered_edges.size());
    assert(!has_duplicates(edges));
}

template <typename I>
void filter_out_loops(EdgeSoup<I>& edges)
{
    const auto past_kept_edges = std::remove_if(std::begin(edges), std::end(edges), &is_loop<I>);
    edges.erase(past_kept_edges, std::end(edges));
    assert(!has_loop_edge(edges));
}

template <typename I>
void filter_out_duplicates_and_loops(EdgeSoup<I>& edges)
{
    std::set<Edge<I>> ordered_edges;
    const auto past_kept_edges = std::remove_if(std::begin(edges), std::end(edges), [&ordered_edges](const Edge<I>& e) {
        return is_loop(e) || !ordered_edges.insert(ordered_edge(e)).second;
    });
    edges.erase(past_kept_edges, std::end(edges));
    assert(edges.size() == ordered_edges.size());
    assert(is_valid(edges));
}

template <typename I>
std::size_t nb_vertices(const EdgeSoup<I>& edges)
{
    return to_vertex_set(edges).size();
}

template <typename I>
std::size_t nb_vertices(const Path<I>& path)
{
    return to_vertex_set(path).size();
}

template <typename I>
std::size_t nb_vertices(const TriangleSoup<I>& triangles)
{
    return to_vertex_set(triangles).size();
}

template <typename I>
std::size_t nb_edges(const EdgeSoup<I>& edges)
{
    assert(is_valid(edges));
    return edges.size();
}

template <typename I>
std::size_t nb_edges(const Path<I>& path)
{
    assert(is_valid(path));
    const auto sz = path.vertices.size();
    return path.closed ? (sz > 2 ? sz : 0) : (sz > 0 ? sz - 1 : 0);
}

template <typename I>
std::size_t nb_edges(const TriangleSoup<I>& triangles)
{
    assert(is_valid(triangles));
    std::set<Edge<I>> ordered_edges;
    for (const auto& t : triangles)
        for (const auto& e : t.edges())
            ordered_edges.insert(ordered_edge(e));
    return ordered_edges.size();
}

template <typename I>
std::pair<I, I> minmax_indices(const EdgeSoup<I>& edges)
{
    assert(!edges.empty());
    auto it_edge = std::cbegin(edges);
    std::pair<I, I> result = std::minmax(it_edge->first, it_edge->second);
    while(++it_edge != std::cend(edges))
    {
        stdutils::minmax_update(result, it_edge->first);
        stdutils::minmax_update(result, it_edge->second);
    }
    return result;
}

template <typename I>
std::pair<I, I> minmax_indices(const Path<I>& path)
{
    assert(!path.vertices.empty());
    const auto pair_it = std::minmax_element(path.vertices);
    return std::make_pair<I, I>(*pair_it.first, *pair_it.second);
}

template <typename I>
std::pair<I, I> minmax_indices(const TriangleSoup<I>& triangles)
{
    assert(!triangles.empty());
    auto it_triangle = std::cbegin(triangles);
    std::pair<I, I> result(*it_triangle[0], *it_triangle[0]);
    while(++it_triangle != std::cend(triangles))
    {
        stdutils::minmax_update(result, *it_triangle[0]);
        stdutils::minmax_update(result, *it_triangle[1]);
        stdutils::minmax_update(result, *it_triangle[2]);
    }
    return result;
}

namespace details
{
    template <typename I>
    class RemapIndices
    {
    public:
        RemapIndices(const std::pair<I, I>& minmax)
            : min_idx(minmax.first)
            , max_idx(minmax.second)
            , idx_map(2u + max_idx - min_idx, I{0})
        {
            assert(min_idx <= max_idx);
        }

        void visit(I idx)
        {
            assert(min_idx <= idx && idx <= max_idx);
            idx_map[1u + idx - min_idx] = 1;     // Notice the +1 extra offset
        }

        void remap()
        {
            std::partial_sum(idx_map.cbegin(), idx_map.cend(), idx_map.begin(), [](I a, I b) -> I { return a + b; });
            assert(idx_map.front() == 0);
        }

        // Call remap() first
        I nb_vertices()
        {
            return idx_map.back();
        }

        // Call remap() first
        I operator[](I idx)
        {
            assert(min_idx <= idx && idx <= max_idx);
            return idx_map[idx - min_idx];
        }
    private:
        I min_idx;
        I max_idx;
        std::vector<I> idx_map;
    };
}

template <typename I>
void remap_indices(EdgeSoup<I>& edges)
{
    details::RemapIndices<I> idx_map(minmax_indices(edges));
    for (const auto& e : edges)
    {
        idx_map.visit(e.first);
        idx_map.visit(e.second);
    }
    idx_map.remap();
    for (auto& e : edges)
    {
        e.first = idx_map[e.first];
        e.second = idx_map[e.second];
    }
    assert(nb_vertices(edges) == idx_map.nb_vertices());
}

template <typename I>
void remap_indices(Path<I>& path)
{
    details::RemapIndices<I> idx_map(minmax_indices(path));
    for (const auto& v : path.vertices)
        idx_map.visit(v);
    idx_map.remap();
    for (auto& v : path.vertices)
        v = idx_map[v];
    assert(nb_vertices(path) == idx_map.nb_vertices());
}

template <typename I>
void remap_indices(TriangleSoup<I>& triangles)
{
    details::RemapIndices<I> idx_map(minmax_indices(triangles));
    for (const auto& t : triangles)
    {
        idx_map.visit(t[0]);
        idx_map.visit(t[1]);
        idx_map.visit(t[2]);
    }
    idx_map.remap();
    for (auto& t : triangles)
    {
        t[0] = idx_map[t[0]];
        t[1] = idx_map[t[1]];
        t[2] = idx_map[t[2]];
    }
    assert(nb_vertices(triangles) == idx_map.nb_vertices());
}

template <typename I>
VertexSet<I> to_vertex_set(const EdgeSoup<I>& edges)
{
    assert(is_valid(edges));
    VertexSet<I> result;
    for (const auto& e : edges)
    {
        result.insert(e.first);
        result.insert(e.second);
    }
    return result;
}

template <typename I>
VertexSet<I> to_vertex_set(const Path<I>& path)
{
    assert(is_valid(path));
    VertexSet<I> result;
    for (const auto& v : path.vertices)
        result.insert(v);
    return result;
}

template <typename I>
VertexSet<I> to_vertex_set(const TriangleSoup<I>& triangles)
{
    assert(is_valid(triangles));
    VertexSet<I> result;
    for (const auto& t : triangles)
    {
        result.insert(t[0]);
        result.insert(t[1]);
        result.insert(t[2]);
    }
    return result;
}

template <typename I>
EdgeSoup<I> to_edge_soup(const Path<I>& path)
{
    assert(is_valid(path));
    EdgeSoup<I> result;
    result.reserve(nb_edges(path));
    for (auto it = path.vertices.cbegin(); it != path.vertices.cend(); it++)
    {
        const auto next = it + 1;
        if (next != path.vertices.cend()) { result.emplace_back(*it, *next); }
        else if (path.closed) { result.emplace_back(*it, path.vertices.front()); }
    }
    assert(result.size() == nb_edges(path));
    assert(is_valid(result));
    return result;
}

template <typename I>
EdgeSoup<I> to_edge_soup(const TriangleSoup<I>& triangles)
{
    assert(is_valid(triangles));
    std::set<Edge<I>> ordered_edges;
    for (const auto& t : triangles)
        for (const auto& e : t.edges())
            ordered_edges.insert(ordered_edge(e));
    auto result = EdgeSoup<I>(ordered_edges.cbegin(), ordered_edges.cend());
    assert(is_valid(result));
    return result;
}

namespace details
{
    // Return a vector of pairs <I index, std::size_t vertex_degree>
    template <typename I>
    std::vector<std::pair<I, std::size_t>> vertex_degree(const EdgeSoup<I>& edges)
    {
        assert(is_valid(edges));
        const auto [min_idx, max_idx] = minmax_indices(edges);
        std::vector<std::pair<I, std::size_t>> result(1 + max_idx - min_idx, std::pair<I, std::size_t>(0, 0));
        I idx = static_cast<I>(min_idx);
        std::for_each(result.begin(), result.end(), [&idx](auto& pair) { pair.first = idx++; });
        for (const auto& e : edges)
        {
            result[e.first  - min_idx].second++;
            result[e.second - min_idx].second++;
        }
        const auto valid_degrees_end = std::remove_if(std::begin(result), std::end(result), [](const auto& pair) { return pair.second == 0; });
        result.erase(valid_degrees_end, result.end());
        return result;
    }

    template <typename I>
    std::size_t nb_edges(const std::vector<Path<I>>& paths)
    {
        return std::accumulate(paths.cbegin(), paths.cend(), std::size_t(0), [](const std::size_t n, const Path<I>& p) -> std::size_t { return n + nb_edges(p); });
    }

    template <typename I>
    class AdjList
    {
    public:
        static constexpr I Undef = IndexTraits<I>::undef();

        // If the degree of the graph is elready known, pass it as a parameter. Else leave the default value and it will be computed.
        AdjList(const EdgeSoup<I>& edges, std::size_t max_deg = 0u)
            : m_adj()
            , m_min_idx(0)
            , m_max_idx(0)
            , m_max_deg(max_deg)
        {
            assert(is_valid(edges));
            assert(!edges.empty());
            std::tie(m_min_idx, m_max_idx) = minmax_indices(edges);
            assert(m_max_idx <= IndexTraits<I>::max_valid_index());
            const std::size_t sz = 1u + m_max_idx - m_min_idx;
            if (m_max_deg == 0u) { m_max_deg = max_degree(edges); }
            assert(m_max_deg != 0u);
            m_adj.resize(sz * m_max_deg, Undef);
            m_visited.resize(sz);
            const auto init_half_edge = [this](I from, I to) {
                assert(m_min_idx <= from && from <= m_max_idx);
                const auto from_rel_idx = static_cast<std::size_t>(from) - m_min_idx;
                assert(m_visited[from_rel_idx].m_unvisited < m_max_deg);
                m_adj[m_max_deg * from_rel_idx + m_visited[from_rel_idx].m_unvisited++] = to;
            };
            for (const auto& e : edges)
            {
                if (e.first == e.second) { assert(0); continue; }
                init_half_edge(e.first, e.second);
                init_half_edge(e.second, e.first);
            }
            for (auto& vis: m_visited) { vis.m_degree = vis.m_unvisited; }
        }

        void reset_visited()
        {
            for (auto& vis: m_visited) { vis.m_unvisited = vis.m_degree; }
        }

        std::size_t degree(I from) const
        {
            const auto rel_idx = static_cast<std::size_t>(from) - m_min_idx;
            return m_visited[rel_idx].m_degree;
        }

        std::size_t unvisited(I from) const
        {
            const auto rel_idx = static_cast<std::size_t>(from) - m_min_idx;
            return m_visited[rel_idx].m_unvisited;
        }

        std::size_t find_sub_idx(I from, I to) const
        {
            const auto rel_idx = static_cast<std::size_t>(from) - m_min_idx;
            std::size_t sub_idx = 0u;
            while(sub_idx < m_max_deg && m_adj[m_max_deg * rel_idx + sub_idx] != to) { sub_idx++; }
            return sub_idx;
        }

        void visit(I from, std::size_t to_sub_idx)
        {
            assert(m_min_idx <= from && from <= m_max_idx);
            const auto rel_idx = static_cast<std::size_t>(from) - m_min_idx;
            if (m_visited[rel_idx].m_unvisited == 0) { assert(0); return; }
            assert(to_sub_idx < m_visited[rel_idx].m_unvisited);
            if (to_sub_idx < m_visited[rel_idx].m_unvisited - 1)
            {
                std::swap(m_adj[m_max_deg * rel_idx + to_sub_idx], m_adj[m_max_deg * rel_idx + m_visited[rel_idx].m_unvisited - 1]);
            }
            // Else, nothing to do, the visited vertex is already at the end of the list
            m_visited[rel_idx].m_unvisited--;
        }

        const I& adj_vertex(I from, std::size_t to_sub_idx) const
        {
            assert(to_sub_idx < m_max_deg);
            assert(m_min_idx <= from && from <= m_max_idx);
            const auto rel_idx = static_cast<std::size_t>(from) - m_min_idx;
            return m_adj[m_max_deg * rel_idx + to_sub_idx];
        }
    private:
        struct DegVisited
        {
            std::size_t m_unvisited = 0u;
            std::size_t m_degree = 0u;
        };
        std::vector<I> m_adj;
        std::vector<DegVisited> m_visited;
        std::size_t m_min_idx;
        std::size_t m_max_idx;
        std::size_t m_max_deg;
    };
} // details namespace

template <typename I>
std::size_t min_degree(const EdgeSoup<I>& edges)
{
    if (edges.empty()) { return 0; }
    const auto vertex_deg = details::vertex_degree(edges);
    assert(!vertex_deg.empty());
    return std::min_element(std::cbegin(vertex_deg), std::cend(vertex_deg), [](const auto& a, const auto& b) { return a.second < b.second; })->second;
}

template <typename I>
std::size_t max_degree(const EdgeSoup<I>& edges)
{
    if (edges.empty()) { return 0; }
    const auto vertex_deg = details::vertex_degree(edges);
    assert(!vertex_deg.empty());
    return std::max_element(std::cbegin(vertex_deg), std::cend(vertex_deg), [](const auto& a, const auto& b) { return a.second < b.second; })->second;
}

template <typename I>
std::pair<std::size_t, std::size_t> minmax_degree(const EdgeSoup<I>& edges)
{
    if (edges.empty()) { return std::pair<std::size_t, std::size_t>(0, 0); }
    const auto vertex_deg = details::vertex_degree(edges);
    assert(!vertex_deg.empty());
    const auto pair_it = std::minmax_element(std::cbegin(vertex_deg), std::cend(vertex_deg), [](const auto& a, const auto& b) { return a.second < b.second; });
    return std::make_pair(pair_it.first->second, pair_it.second->second);
}

template <typename I>
std::vector<Path<I>> extract_paths(const EdgeSoup<I>& edges)
{
    assert(is_valid(edges));
    std::vector<Path<I>> result;
    if (edges.empty())
        return result;

    std::vector<std::pair<I, std::size_t>> degrees = details::vertex_degree(edges);
    const auto max_deg = std::max_element(std::cbegin(degrees), std::cend(degrees), [](const auto& a, const auto& b) { return a.second < b.second; })->second;

    // All vertices of deg != 2 are endpoints, and we shall process those ones first. The remaining vertices of deg = 2 are part of closed 1-manifold paths.
    std::partition(degrees.begin(), degrees.end(), [](const auto& pair_idx_deg){ return pair_idx_deg.second != 2; });

    // Create adjency list
    details::AdjList<I> adj_list(edges, max_deg);

    for (const auto& [from, deg] : degrees)
    {
        UNUSED(deg);
        while (adj_list.unvisited(from) > 0)
        {
            Path<I> path;
            I idx = from;
            bool closed_the_loop = false;
            bool next_is_an_endpoint = false;
            while (!(closed_the_loop || next_is_an_endpoint))
            {
                path.vertices.emplace_back(idx);
                const auto next_idx = adj_list.adj_vertex(idx, 0);
                adj_list.visit(idx, 0);
                const auto sub_idx = adj_list.find_sub_idx(next_idx, idx);
                adj_list.visit(next_idx, sub_idx);
                closed_the_loop = (next_idx == from);
                next_is_an_endpoint = (adj_list.degree(next_idx) != 2);
                // From graph theory: Either we exit the loop or the next vertex is traversable
                assert(closed_the_loop || next_is_an_endpoint || adj_list.unvisited(next_idx) > 0);
                idx = next_idx;
            }
            if (idx == from)
            {
                path.closed = true;
            }
            else
            {
                path.closed = false;
                path.vertices.emplace_back(idx);
            }
            assert(path.vertices.size() > 1);
            assert(is_valid(path));
            result.emplace_back(std::move(path));
        }
    }

    assert(details::nb_edges(result) == nb_edges(edges));
    return result;
}


} // namespace graphs