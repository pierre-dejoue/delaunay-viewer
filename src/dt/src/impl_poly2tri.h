// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <graphs/graph.h>
#include <dt/dt_interface.h>
#include <poly2tri/poly2tri.h>

#include <cstdint>
#include <exception>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <utility>
#include <vector>

#ifndef DT_POLY2TRI_ORIGINAL_API
#define DT_POLY2TRI_ORIGINAL_API 0
#endif

namespace delaunay {

template <typename F, typename I = std::uint32_t>
class Poly2triImpl : public Interface<F, I>
{
public:
    Poly2triImpl(const stdutils::io::ErrorHandler* err_handler = nullptr);

    void add_path(const shapes::PointPath2d<F>& pp) override;
    void add_hole(const shapes::PointPath2d<F>& pp) override;
    void add_steiner(const shapes::PointCloud2d<F>& pc) override;

private:
    void triangulate_impl(TriangulationPolicy policy, shapes::Triangles2d<F, I>& result) const override;

    shapes::Points2d<F> m_points;
    std::vector<std::pair<I, I>> m_polylines_indices;
    std::vector<bool> m_polyline_is_closed;
    std::vector<std::pair<I, I>> m_steiner_indices;
    bool m_has_main_path;

    using Interface<F, I>::m_err_handler;
};

template <typename F, typename I>
std::unique_ptr<Interface<F, I>> get_poly2tri_impl(const stdutils::io::ErrorHandler* err_handler)
{
    return std::make_unique<Poly2triImpl<F, I>>(err_handler);
}


//
//
// Implementation
//
//


namespace details {
namespace p2t {

    template <typename F>
    std::vector<::p2t::Point> copy_vertices(const shapes::Points2d<F>& points)
    {
        std::vector<::p2t::Point> result;
        result.reserve(points.size());
        for (const auto& p : points) { result.emplace_back(static_cast<double>(p.x), static_cast<double>(p.y)); }
        return result;
    }

#if DT_POLY2TRI_ORIGINAL_API
    template <typename I>
    std::vector<::p2t::Point*> to_polyline(const std::vector<::p2t::Point>& all_points, std::pair<I, I> range)
    {
        const I begin = range.first;
        const I end = range.second;
        std::vector<::p2t::Point*> result;
        assert(begin <= end);
        assert(end <= all_points.size());
        result.resize(end - begin, nullptr);
        std::iota(result.begin(), result.end(), const_cast<::p2t::Point*>(&all_points[begin]));
        return result;
    }
#endif

} // namespace p2t
} // namespace details

template <typename F, typename I>
Poly2triImpl<F, I>::Poly2triImpl(const stdutils::io::ErrorHandler* err_handler)
    : Interface<F,I>(err_handler)
    , m_points()
    , m_polylines_indices()
    , m_steiner_indices()
    , m_has_main_path(false)
{ }

template <typename F, typename I>
void Poly2triImpl<F, I>::add_path(const shapes::PointPath2d<F>& pp)
{
    assert(shapes::is_valid(pp));
#if DT_POLY2TRI_ORIGINAL_API
    if (m_has_main_path)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::ERR, "Only one main polyline is supported. Ignoring this one. Try using add_hole() instead."); }
        return;
    }
    if (!pp.closed && m_err_handler) { m_err_handler(stdutils::io::Severity::WARN, "add_path(): All polylines are interpreted as closed"); }
#else
    if (!pp.closed && pp.vertices.size() < 2)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::ERR, "add_path(): Ignoring an open polyline with less than 2 vertices"); }
        return;
    }
#endif
    if (pp.closed && pp.vertices.size() < 3)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::ERR, "add_path(): Ignoring a closed polyline with less than 3 vertices"); }
        return;
    }

    const I begin_idx = static_cast<I>(m_points.size());
    m_points.reserve(m_points.size() + pp.vertices.size());
    m_points.insert(m_points.end(), pp.vertices.begin(), pp.vertices.end());
    const I end_idx = static_cast<I>(m_points.size());

    // The first path will be the main polyline (next paths are the holes)
    m_polylines_indices.emplace(m_polylines_indices.begin(), begin_idx, end_idx);
    m_polyline_is_closed.emplace_back(pp.closed);

    m_has_main_path = true;
}

template <typename F, typename I>
void Poly2triImpl<F, I>::add_hole(const shapes::PointPath2d<F>& pp)
{
    assert(shapes::is_valid(pp));
#if DT_POLY2TRI_ORIGINAL_API
    if (!pp.closed && m_err_handler) { m_err_handler(stdutils::io::Severity::WARN, "add_hole(): All polylines are interpreted as closed"); }
#else
    if (!pp.closed && pp.vertices.size() < 2)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::ERR, "add_hole(): Ignoring an open polyline with less than 2 vertices"); }
        return;
    }
#endif
    if (pp.closed && pp.vertices.size() < 3)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::ERR, "add_hole(): Ignoring a closed polyline with less than 3 vertices"); }
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
void Poly2triImpl<F, I>::add_steiner(const shapes::PointCloud2d<F>& pc)
{
    const I begin_idx = static_cast<I>(m_points.size());
    m_points.reserve(m_points.size() + pc.vertices.size());
    m_points.insert(m_points.end(), pc.vertices.begin(), pc.vertices.end());
    const I end_idx = static_cast<I>(m_points.size());
    m_steiner_indices.emplace_back(begin_idx, end_idx);
}

template <typename F, typename I>
void Poly2triImpl<F, I>::triangulate_impl(TriangulationPolicy policy, shapes::Triangles2d<F, I>& result) const
{
    if (m_points.size() < 3)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::WARN, "Not enough points to triangulate. The output will be empty."); }
        return;
    }

    if (policy == TriangulationPolicy::CDT && !m_has_main_path)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::WARN, "Missing the outer polyline for the constrained Delaunay triangulation. The output will be empty."); }
        return;
    }

#if DT_POLY2TRI_ORIGINAL_API
    //
    // Original poly2tri API
    //

    if (policy == TriangulationPolicy::PointCloud)
    {
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::WARN, "Unconstrained Delaunay triangulation is not supported. The output will be empty."); }
        return;
    }

    std::vector<p2t::Point> p2t_points = details::p2t::copy_vertices(m_points);

    // As per poly2tri documentation:
    // Initialize CDT with a simple polyline (this defines the constrained edges)
    const auto main_polyline = m_has_main_path ? details::p2t::to_polyline<I>(p2t_points, m_polylines_indices.front()) : std::vector<p2t::Point*>();
    p2t::CDT cdt(main_polyline);

    // Add holes if necessary (also simple polylines)
    for (unsigned int path_idx = 1; path_idx < m_polylines_indices.size(); path_idx++)
    {
        const auto hole_polyline = details::p2t::to_polyline<I>(p2t_points, m_polylines_indices.at(path_idx));
        cdt.AddHole(hole_polyline);
    }

    // Add Steiner points
    for (const auto& range : m_steiner_indices)
    {
        assert(range.second <= p2t_points.size());
        for (auto idx = range.first; idx < range.second; idx++)
            cdt.AddPoint(&p2t_points[idx]);
    }

    // Triangulate
    cdt.Triangulate();
    const std::vector<p2t::Triangle*> p2t_triangles = cdt.GetTriangles();

#else
    //
    // Modern poly2tri API
    //

    std::vector<p2t::Point> p2t_points = details::p2t::copy_vertices(m_points);

    p2t::CDT cdt;

    if (policy == TriangulationPolicy::CDT)
    {
        // As per poly2tri documentation:
        // Initialize CDT with a simple polyline (this defines the constrained edges)
        if(!m_polylines_indices.empty())
        {
            const auto& range = m_polylines_indices.front();
            assert(range.first <= range.second);
            assert(range.second <= p2t_points.size());
            if (m_polyline_is_closed.front())
                cdt.AddPolyline(p2t_points.data() + static_cast<std::size_t>(range.first), static_cast<std::size_t>(range.second - range.first));
            else
                cdt.AddOpenPolyline(p2t_points.data() + static_cast<std::size_t>(range.first), static_cast<std::size_t>(range.second - range.first));
        }

        // Add holes and open polylines
        for (unsigned int path_idx = 1; path_idx < m_polylines_indices.size(); path_idx++)
        {
            const auto& range = m_polylines_indices.at(path_idx);
            assert(range.first <= range.second);
            assert(range.second <= p2t_points.size());
            if (m_polyline_is_closed.at(path_idx))
                cdt.AddHole(p2t_points.data() + static_cast<std::size_t>(range.first), static_cast<std::size_t>(range.second - range.first));
            else
                cdt.AddOpenPolyline(p2t_points.data() + static_cast<std::size_t>(range.first), static_cast<std::size_t>(range.second - range.first));
        }

        // Add Steiner points
        for (const auto& range : m_steiner_indices)
        {
            assert(range.first <= range.second);
            assert(range.second <= p2t_points.size());
            cdt.AddPoints(p2t_points.data() + static_cast<std::size_t>(range.first), static_cast<std::size_t>(range.second - range.first));
        }

        // Triangulate
        cdt.Triangulate(p2t::Policy::OuterPolygon);
    }
    else
    {
        assert(policy == TriangulationPolicy::PointCloud);

        // Add Steiner points
        cdt.AddPoints(p2t_points.data(), p2t_points.size());

        // Triangulate
        cdt.Triangulate(p2t::Policy::ConvexHull);
    }

    const auto& p2t_triangles = cdt.GetTriangles();
#endif

    // Copy result
    result.vertices = m_points;
    const p2t::Point* begin_point = &p2t_points[0];
    result.faces.reserve(p2t_triangles.size());
    const I nb_vertices = static_cast<I>(result.vertices.size());
    for (const auto& triangle : p2t_triangles)
    {
        graphs::Triangle<I> face;
        bool valid_range = true;
        for (unsigned int i = 0; i < 3; i++)
        {
            const p2t::Point* point = triangle->GetPoint(static_cast<int>(i));
            face[i] = static_cast<I>(std::distance(begin_point, point));
            valid_range &= (face[i] < nb_vertices);
        }
        if (valid_range && graphs::is_valid(face)) { result.faces.emplace_back(std::move(face)); }
        else if (m_err_handler) { m_err_handler(stdutils::io::Severity::ERR, "The triangulation process returned an invalid triangle"); }
    }
}

} // namespace delaunay
