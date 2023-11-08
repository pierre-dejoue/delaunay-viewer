#pragma once

#include <graphs/index.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <set>
#include <utility>
#include <vector>


namespace graphs
{

/**
 * Edge
 */
template <typename I = std::uint32_t>
using Edge = std::pair<I, I>;

// Check range of vertex indices
template <typename I>
constexpr bool is_defined(const Edge<I>& edge) { return edge.first != IndexTraits<I>::undef() && edge.second != IndexTraits<I>::undef(); }

// Identify a loop edge
template <typename I>
constexpr bool is_loop(const Edge<I>& edge) { return edge.first == edge.second; }

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
