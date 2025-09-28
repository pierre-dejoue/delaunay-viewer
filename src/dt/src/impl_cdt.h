// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <dt/dt_interface.h>

#include "cdt_wrap.h"

#include <cstdint>
#include <exception>
#include <iterator>
#include <utility>
#include <vector>

namespace delaunay {

// Fc  floating-point type used by the library (computation)
// F   floating-point type used for the interface
template <typename Fc, typename F, typename I = std::uint32_t>
class CDTImpl : public Interface<F, I>
{
public:
    CDTImpl(const stdutils::io::ErrorHandler* err_handler = nullptr);

    void add_path(const shapes::PointPath2d<F>& pp) override;
    void add_hole(const shapes::PointPath2d<F>& pp) override;
    void add_steiner(const shapes::PointCloud2d<F>& pc) override;

private:
    void triangulate_impl(TriangulationPolicy policy, shapes::Triangles2d<F, I>& result) const override;

    shapes::Points2d<F> m_points;
    std::vector<std::pair<I, I>> m_polylines_indices;
    std::vector<bool> m_polylines_closed;

    using Interface<F, I>::m_err_handler;
};

template <typename Fc, typename F, typename I>
std::unique_ptr<Interface<F, I>> get_cdt_impl(const stdutils::io::ErrorHandler* err_handler)
{
    return std::make_unique<CDTImpl<Fc, F, I>>(err_handler);
}


//
//
// Implementation
//
//


namespace details {
namespace cdt {

    template <typename Fc, typename F>
    std::vector<CDT::V2d<Fc>> copy_vertices(const shapes::Points2d<F>& points)
    {
        std::vector<CDT::V2d<Fc>> result;
        result.reserve(points.size());
        std::transform(std::cbegin(points), std::cend(points), std::back_inserter(result), [](const auto& p) {
            return CDT::V2d<Fc>(static_cast<Fc>(p.x), static_cast<Fc>(p.y));
        });
        return result;
    }

} // namespace cdt
} // namespace details

template <typename Fc, typename F, typename I>
CDTImpl<Fc, F, I>::CDTImpl(const stdutils::io::ErrorHandler* err_handler)
    : Interface<F,I>(err_handler)
    , m_points()
    , m_polylines_indices()
    , m_polylines_closed()
{ }

template <typename Fc, typename F, typename I>
void CDTImpl<Fc, F, I>::add_path(const shapes::PointPath2d<F>& pp)
{
    assert(shapes::is_valid(pp));
    if (pp.closed && pp.vertices.size() < 3)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::WARN, "Ignoring a closed polyline with less than 3 vertices"); }
        return;
    }

    const I begin_idx = static_cast<I>(m_points.size());
    m_points.reserve(m_points.size() + pp.vertices.size());
    m_points.insert(m_points.end(), pp.vertices.begin(), pp.vertices.end());
    const I end_idx = static_cast<I>(m_points.size());

    m_polylines_indices.emplace_back(begin_idx, end_idx);
    m_polylines_closed.emplace_back(pp.closed);
}

template <typename Fc, typename F, typename I>
void CDTImpl<Fc, F, I>::add_hole(const shapes::PointPath2d<F>& pp)
{
    add_path(pp);
}

template <typename Fc, typename F, typename I>
void CDTImpl<Fc, F, I>::add_steiner(const shapes::PointCloud2d<F>& pc)
{
    m_points.reserve(m_points.size() + pc.vertices.size());
    m_points.insert(m_points.end(), pc.vertices.begin(), pc.vertices.end());
}

template <typename Fc, typename F, typename I>
void CDTImpl<Fc, F, I>::triangulate_impl(TriangulationPolicy policy, shapes::Triangles2d<F, I>& result) const
{
    if (m_points.size() < 3)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::WARN, "Not enough points to triangulate. The output will be empty."); }
        return;
    }

    CDT::Triangulation<Fc> cdt;
    std::vector<CDT::V2d<Fc>> vertices = details::cdt::copy_vertices<Fc, F>(m_points);
    cdt.insertVertices(vertices);
    CDT::EdgeVec edges;
    if (policy == TriangulationPolicy::CDT)
    {
        assert(m_polylines_indices.size() == m_polylines_closed.size());
        std::size_t polyline_idx = 0;
        for (const auto& [begin, end] : m_polylines_indices)
        {
            assert(begin <= end);
            edges.reserve(edges.size() + (end - begin));
            for (I idx = begin; idx < (end - 1); idx++)
            {
                edges.emplace_back(static_cast<CDT::VertInd>(idx), static_cast<CDT::VertInd>(idx + 1));
            }
            if (m_polylines_closed.at(polyline_idx++))
            {
                assert(begin != (end-1));        // closed polylines with size < 3 are rejected in add_path/add_hole
                edges.emplace_back(static_cast<CDT::VertInd>(end - 1), static_cast<CDT::VertInd>(begin));
            }
        }
        cdt.insertEdges(edges);
    }
    if (edges.empty())
    {
        // Delaunay triangulation of a point cloud
        cdt.eraseSuperTriangle();
    }
    else
    {
        // Constrained Delaunay triangulation
        cdt.eraseOuterTrianglesAndHoles();
    }
    const auto cdt_triangles = cdt.triangles;

    result.vertices = m_points;
    result.faces.reserve(cdt_triangles.size());
    for (const auto& triangle : cdt_triangles)
    {
        result.faces.emplace_back(
            static_cast<I>(triangle.vertices[0]),
            static_cast<I>(triangle.vertices[1]),
            static_cast<I>(triangle.vertices[2])
        );
    }
}

} // namespace delaunay
