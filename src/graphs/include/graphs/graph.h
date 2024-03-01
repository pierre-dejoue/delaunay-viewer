// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <graphs/index.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <set>
#include <utility>
#include <vector>

namespace graphs {

/**
 * Edge
 */
template <typename I = std::uint32_t>
class Edge
{
public:
    using index = I;
    static constexpr I undef() { return IndexTraits<I>::undef(); }

    // Ctors
    constexpr Edge() : m_pair(undef(), undef()) {}
    constexpr Edge(I orig, I dest) : m_pair(orig, dest) {}
    template <typename U>
    explicit constexpr Edge(const std::pair<U, U>& pair) : m_pair(pair) {}
    Edge(const Edge&) = default;
    Edge& operator=(const Edge&) = default;
    Edge(Edge&&) = default;
    Edge& operator=(Edge&&) = default;

    // The vertices can be accessed with edge[0] and edge[1]
    constexpr const I& operator[](std::uint8_t idx) const { assert(idx < 2); return idx == 0u ? m_pair.first : m_pair.second; }
    constexpr I& operator[](std::uint8_t idx)             { assert(idx < 2); return idx == 0u ? m_pair.first : m_pair.second; }

    // The vertices can be accessed with orig() and dest()
    constexpr const I& orig() const { return m_pair.first; }
    constexpr I& orig()             { return m_pair.first; }
    constexpr const I& dest() const { return m_pair.second; }
    constexpr I& dest()             { return m_pair.second; }

private:
    std::pair<I, I> m_pair;
};

template <typename I>
constexpr bool operator==(const Edge<I>& lhs, const Edge<I>& rhs) { return lhs.orig() == rhs.orig() && lhs.dest() == rhs.dest(); }

template <typename I>
constexpr bool operator!=(const Edge<I>& lhs, const Edge<I>& rhs) { return !(lhs == rhs); }

template <typename I>
constexpr bool operator<(const Edge<I>& lhs, const Edge<I>& rhs) { return lhs.orig() < rhs.orig() || (lhs.orig() == rhs.orig() && lhs.dest() < rhs.dest()); }

// Check range of vertex indices
template <typename I>
constexpr bool is_defined(const Edge<I>& edge) { return edge.orig() != IndexTraits<I>::undef() && edge.dest() != IndexTraits<I>::undef(); }

// Identify a loop edge
template <typename I>
constexpr bool is_loop(const Edge<I>& edge) { return edge.orig() == edge.dest(); }

/**
 * Edge Soup
 */
template <typename I = std::uint32_t>
using EdgeSoup = std::vector<Edge<I>>;

/**
 * Path
 */
template <typename I = std::uint32_t>
struct Path
{
    Path() : closed(false), vertices() {}
    bool closed;
    std::vector<I> vertices;
};

/**
 * Triangle
 */
template <typename I = std::uint32_t>
class Triangle
{
public:
    using index = I;
    constexpr Triangle() : vertices{ I(), I(), I() } {}
    constexpr Triangle(const I& a, const I& b, const I& c) : vertices{a, b, c} {}
    constexpr I& operator[](std::size_t idx) { assert(idx < 3); return vertices[idx]; }
    constexpr const I& operator[](std::size_t idx) const { assert(idx < 3); return vertices[idx]; }
    constexpr std::array<Edge<I>, 3> edges() const { return { Edge<I>(vertices[0], vertices[1]), Edge<I>(vertices[1], vertices[2]), Edge<I>(vertices[2], vertices[0]) }; }
private:
    std::array<I, 3> vertices;
};

/**
 * Triangle Soup
 */
template <typename I = std::uint32_t>
using TriangleSoup = std::vector<Triangle<I>>;

/**
 * Set of vertices
 */
template <typename I = std::uint32_t>
using VertexSet = std::set<I>;

} // namespace graphs
