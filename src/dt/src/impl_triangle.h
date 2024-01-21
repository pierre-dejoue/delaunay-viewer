// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <graphs/graph.h>
#include <dt/dt_interface.h>
#include <triangle.h>

#include <array>
#include <cstdint>
#include <exception>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <utility>
#include <vector>

namespace delaunay {

template <typename F, typename I = std::uint32_t>
class TriangleImpl : public Interface<F, I>
{
public:
    TriangleImpl(const stdutils::io::ErrorHandler* err_handler = nullptr);

    void add_path(const shapes::PointPath2d<F>& pp) override;
    void add_hole(const shapes::PointPath2d<F>& pp) override;
    void add_steiner(const shapes::PointCloud2d<F>& pc) override;

private:
    void triangulate_impl(TriangulationPolicy policy, shapes::Triangles2d<F, I>& result) const override;

    shapes::Points2d<F> m_points;
    std::vector<std::pair<I, I>> m_polylines_indices;
    std::vector<bool> m_polyline_is_closed;

    using Interface<F, I>::m_err_handler;
};

template <typename F, typename I>
std::unique_ptr<Interface<F, I>> get_triangle_impl(const stdutils::io::ErrorHandler* err_handler)
{
    return std::make_unique<TriangleImpl<F, I>>(err_handler);
}


//
//
// Implementation
//
//


namespace details {
namespace triangle {

    template <typename F>
    using Point = std::array<F, 2>;

    using Edge = std::array<int, 2>;

    template <typename F>
    std::vector<Point<F>> copy_vertices(const shapes::Points2d<F>& points)
    {
        std::vector<Point<F>> result;
        result.reserve(points.size());
        std::transform(std::cbegin(points), std::cend(points), std::back_inserter(result), [](const auto&p) { return Point<F> { p.x, p.y }; });
        return result;
    }

    void reset(triangulateio& tri)
    {
        tri.pointlist = nullptr;
        tri.pointattributelist = nullptr;
        tri.pointmarkerlist = nullptr;
        tri.numberofpoints = 0;
        tri.numberofpointattributes = 0;
        tri.trianglelist = nullptr;
        tri.triangleattributelist = nullptr;
        tri.trianglearealist = nullptr;
        tri.neighborlist = nullptr;
        tri.numberoftriangles = 0;
        tri.numberofcorners = 0;
        tri.numberoftriangleattributes = 0;
        tri.segmentlist = nullptr;
        tri.segmentmarkerlist = nullptr;
        tri.numberofsegments = 0;
        tri.holelist = nullptr;
        tri.numberofholes = 0;
        tri.regionlist = nullptr;
        tri.numberofregions = 0;
        tri.edgelist = nullptr;
        tri.edgemarkerlist = nullptr;
        tri.normlist = nullptr;
        tri.numberofedges = 0;
    }

    // Set to null the pointers allocated by the client BEFORE calling this function.
    // All the remaining non-nullptr should have been allocated by Triangle.
    void free_all(triangulateio& tri)
    {
        if (tri.pointlist != nullptr) { ::free(tri.pointlist); tri.pointlist = nullptr; }
        if (tri.pointattributelist != nullptr) { ::free(tri.pointattributelist); tri.pointattributelist = nullptr; }
        if (tri.pointmarkerlist != nullptr) { ::free(tri.pointmarkerlist); tri.pointmarkerlist = nullptr; }
        if (tri.trianglelist != nullptr) { ::free(tri.trianglelist); tri.trianglelist = nullptr; }
        if (tri.triangleattributelist != nullptr) { ::free(tri.triangleattributelist); tri.triangleattributelist = nullptr; }
        if (tri.trianglearealist != nullptr) { ::free(tri.trianglearealist); tri.trianglearealist = nullptr; }
        if (tri.neighborlist != nullptr) { ::free(tri.neighborlist); tri.neighborlist = nullptr; }
        if (tri.segmentlist != nullptr) { ::free(tri.segmentlist); tri.segmentlist = nullptr; }
        if (tri.segmentmarkerlist != nullptr) { ::free(tri.segmentmarkerlist); tri.segmentmarkerlist = nullptr; }
        if (tri.holelist != nullptr) { ::free(tri.holelist); tri.holelist = nullptr; }
        if (tri.regionlist != nullptr) { ::free(tri.regionlist); tri.regionlist = nullptr; }
        if (tri.edgelist != nullptr) { ::free(tri.edgelist); tri.edgelist = nullptr; }
        if (tri.edgemarkerlist != nullptr) { ::free(tri.edgemarkerlist); tri.edgemarkerlist = nullptr; }
        if (tri.normlist != nullptr) { ::free(tri.normlist); tri.normlist = nullptr; }
    }

} // namespace triangle
} // namespace details

template <typename F, typename I>
TriangleImpl<F, I>::TriangleImpl(const stdutils::io::ErrorHandler* err_handler)
    : Interface<F,I>(err_handler)
    , m_points()
    , m_polylines_indices()
    , m_polyline_is_closed()
{ }

template <typename F, typename I>
void TriangleImpl<F, I>::add_path(const shapes::PointPath2d<F>& pp)
{
    assert(shapes::is_valid(pp));
    if (pp.closed && pp.vertices.size() < 3)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::ERR, "add_path(): Ignoring a closed polyline with less than 3 vertices"); }
        return;
    }

    const I begin_idx = static_cast<I>(m_points.size());
    m_points.reserve(m_points.size() + pp.vertices.size());
    m_points.insert(m_points.end(), pp.vertices.begin(), pp.vertices.end());
    const I end_idx = static_cast<I>(m_points.size());

    m_polylines_indices.emplace_back(begin_idx, end_idx);
    m_polyline_is_closed.emplace_back(pp.closed);
}

template <typename F, typename I>
void TriangleImpl<F, I>::add_hole(const shapes::PointPath2d<F>& pp)
{
    add_path(pp);
    // TODO the Triangle library can handle holes
}

template <typename F, typename I>
void TriangleImpl<F, I>::add_steiner(const shapes::PointCloud2d<F>& pc)
{
    m_points.reserve(m_points.size() + pc.vertices.size());
    m_points.insert(m_points.end(), pc.vertices.begin(), pc.vertices.end());
}

template <typename F, typename I>
void TriangleImpl<F, I>::triangulate_impl(TriangulationPolicy policy, shapes::Triangles2d<F, I>& result) const
{
    if (m_points.size() < 3)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::WARN, "Not enough points to triangulate. The output will be empty."); }
        return;
    }

    struct triangulateio in, out;
    details::triangle::reset(in);
    details::triangle::reset(out);

    // Q: Quiet. Suppresses all explanation of what Triangle is doing, unless an error occurs.
    // z: Index everything from zero
    std::string options = "Qz";

    std::vector<details::triangle::Point<F>> vertices = details::triangle::copy_vertices(m_points);
    assert(!vertices.empty());
    in.numberofpoints = static_cast<int>(vertices.size());
    in.pointlist = vertices[0].data();

    std::vector<details::triangle::Edge> edges;
    if (policy == TriangulationPolicy::CDT)
    {
        // p: PSLG (Planar Straight Line Graph) triangulate a point set with constrained edges
        options.insert(options.begin(), 'p');

        assert(m_polylines_indices.size() == m_polyline_is_closed.size());
        std::size_t polyline_idx = 0;
        for (const auto& [begin, end] : m_polylines_indices)
        {
            assert(begin <= end);
            edges.reserve(edges.size() + (end - begin));
            for (I idx = begin; idx < (end - 1); idx++)
            {
                edges.emplace_back(details::triangle::Edge{ static_cast<int>(idx), static_cast<int>(idx + 1) });
            }
            if (m_polyline_is_closed.at(polyline_idx++))
            {
                assert(begin != (end-1));        // closed polylines with size < 3 are rejected in add_path/add_hole
                edges.emplace_back(details::triangle::Edge{ static_cast<int>(end - 1), static_cast<int>(begin) });
            }
        }
    }
    in.numberofsegments = static_cast<int>(edges.size());
    in.segmentlist = edges.empty() ? nullptr : edges[0].data();

    // Triangulate
    ::triangulate(options.data(), &in, &out, nullptr);

    // Copy result
    result.vertices = m_points;
    assert(out.numberoftriangles == 0 || out.trianglelist != nullptr);
    assert(out.numberoftriangles >= 0);
    result.faces.reserve(static_cast<std::size_t>(out.numberoftriangles));
    for (auto idx = 0; idx < out.numberoftriangles; idx++)
    {
        result.faces.emplace_back(
            static_cast<I>(out.trianglelist[3 * idx + 0]),
            static_cast<I>(out.trianglelist[3 * idx + 1]),
            static_cast<I>(out.trianglelist[3 * idx + 2])
        );
    }

    // Free resources allocated by the Triangle library
    in.pointlist = nullptr;
    in.segmentlist = nullptr;
    details::triangle::free_all(in);
    details::triangle::free_all(out);
}

} // namespace delaunay
