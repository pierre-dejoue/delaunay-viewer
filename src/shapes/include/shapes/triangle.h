#pragma once

#include <shapes/point.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <set>
#include <vector>


namespace shapes
{

/**
 * Triangles
 *
 * A triangle soup
 */
template <typename P, typename I = std::uint32_t>
struct Triangles
{
    static constexpr int dim = P::dim;
    using scalar = typename P::scalar;
    using index = I;
    using face = std::array<I, 3>;
    Triangles() : vertices(), faces() {}
    std::vector<P> vertices;
    std::vector<face> faces;
};

template <typename F, typename I = std::uint32_t>
using Triangles2d = Triangles<Point2d<F>, I>;

template <typename F, typename I = std::uint32_t>
using Triangles3d = Triangles<Point3d<F>, I>;

template <typename P, typename I>
bool is_valid(const Triangles<P, I>& ts)
{
    const I nb_vertices = static_cast<I>(ts.vertices.size());
    return std::all_of(std::begin(ts.faces), std::end(ts.faces), [nb_vertices](const Triangles<P>::face& f) {
        return f[0] < nb_vertices && f[1] < nb_vertices && f[2] < nb_vertices && f[0] != f[1] && f[1] != f[2] && f[2] != f[0];
    });
}

template <typename P, typename I>
std::size_t nb_edges(const Triangles<P, I>& ts)
{
    assert(is_valid(ts));
    using OrderedEdge = std::pair<I, I>;
    std::set<OrderedEdge> edges;
    for (const auto& t : ts.faces)
    {
        edges.insert(std::minmax(t[0], t[1]));
        edges.insert(std::minmax(t[1], t[2]));
        edges.insert(std::minmax(t[2], t[0]));
    }
    return edges.size();
}

} // namespace shapes
