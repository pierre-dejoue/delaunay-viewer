#pragma once

#include <dt/dt_interface.h>
#include <poly2tri/poly2tri.h>

#include <cstdint>
#include <exception>
#include <iterator>
#include <numeric>
#include <sstream>
#include <utility>
#include <vector>


namespace delaunay
{

template <typename F, typename I = std::uint32_t>
class Poly2triImpl : public Interface<F, I>
{
public:
    Poly2triImpl();

    void add_path(const shapes::PointPath2d<F>& pp, const stdutils::io::ErrorHandler& err_handler) override;
    void add_hole(const shapes::PointPath2d<F>& pp, const stdutils::io::ErrorHandler& err_handler) override;
    void add_steiner(const shapes::PointCloud2d<F>& pc, const stdutils::io::ErrorHandler& err_handler) override;

    shapes::Triangles2d<F, I> triangulate(const stdutils::io::ErrorHandler& err_handler) const override;

private:
    std::vector<shapes::Point2d<F>> m_points;
    std::vector<std::pair<I, I>> m_polylines_indices;
    std::vector<std::pair<I, I>> m_steiner_indices;
    bool has_main_path = false;
};

template <typename F, typename I>
std::unique_ptr<Interface<F, I>> get_poly2tri_impl()
{
    return std::make_unique<Poly2triImpl<F, I>>();
}

//
// Implementation
//
namespace
{
    template <typename F>
    std::vector<p2t::Point> copy_p2t_vertices(const std::vector<shapes::Point2d<F>>& points)
    {
        std::vector<p2t::Point> result;
        result.reserve(points.size());
        for (const auto& p : points) { result.emplace_back(static_cast<double>(p.x), static_cast<double>(p.y)); }
        return result;
    }
}

template <typename F, typename I>
Poly2triImpl<F, I>::Poly2triImpl() = default;

template <typename F, typename I>
void Poly2triImpl<F, I>::add_path(const shapes::PointPath2d<F>& pp, const stdutils::io::ErrorHandler& err_handler)
{
    if (has_main_path)
    {
        err_handler(stdutils::io::Severity::ERROR, "Poly2tri only supports one polyline. Ignoring this one");
        return;
    }
    if (pp.vertices.size() < 3)
    {
        err_handler(stdutils::io::Severity::WARNING, "Ignored polyline with less than 3 vertices");
        return;
    }
    if (!pp.closed) { err_handler(stdutils::io::Severity::WARNING, "Poly2tri interpret all polylines as closed"); }

    const I begin_idx = static_cast<I>(m_points.size());
    m_points.reserve(m_points.size() + pp.vertices.size());
    m_points.insert(m_points.end(), pp.vertices.begin(), pp.vertices.end());
    const I end_idx = static_cast<I>(m_points.size());

    // The first path will be the main polyline (next paths are the holes)
    m_polylines_indices.emplace(m_polylines_indices.begin(), begin_idx, end_idx);

    has_main_path = true;
}

template <typename F, typename I>
void Poly2triImpl<F, I>::add_hole(const shapes::PointPath2d<F>& pp, const stdutils::io::ErrorHandler& err_handler)
{
    if (pp.vertices.size() < 3)
    {
        err_handler(stdutils::io::Severity::WARNING, "Ignored polyline with less than 3 vertices");
        return;
    }
    if (!pp.closed) { err_handler(stdutils::io::Severity::WARNING, "Poly2tri interpret all polylines as closed"); }
    const I begin_idx = static_cast<I>(m_points.size());
    m_points.reserve(m_points.size() + pp.vertices.size());
    m_points.insert(m_points.end(), pp.vertices.begin(), pp.vertices.end());
    const I end_idx = static_cast<I>(m_points.size());
    m_polylines_indices.emplace_back(begin_idx, end_idx);
}

template <typename F, typename I>
void Poly2triImpl<F, I>::add_steiner(const shapes::PointCloud2d<F>& pc, const stdutils::io::ErrorHandler& err_handler)
{
    const I begin_idx = static_cast<I>(m_points.size());
    m_points.reserve(m_points.size() + pc.vertices.size());
    m_points.insert(m_points.end(), pc.vertices.begin(), pc.vertices.end());
    const I end_idx = static_cast<I>(m_points.size());
    m_steiner_indices.emplace_back(begin_idx, end_idx);
}

namespace
{
    template <typename I>
    std::vector<p2t::Point*> to_p2t_polyline(const std::vector<p2t::Point>& all_points, std::pair<I, I> range)
    {
        const I begin = range.first;
        const I end = range.second;
        std::vector<p2t::Point*> result;
        assert(begin <= end);
        assert(end <= all_points.size());
        result.resize(end - begin, nullptr);
        std::iota(result.begin(), result.end(), const_cast<p2t::Point*>(&all_points[begin]));
        return result;
    }
}

template <typename F, typename I>
shapes::Triangles2d<F, I> Poly2triImpl<F, I>::triangulate(const stdutils::io::ErrorHandler& err_handler) const
{
    shapes::Triangles2d<F, I> result;
    if (!has_main_path)
    {
        err_handler(stdutils::io::Severity::ERROR, "Missing the main polyline. The output will be empty.");
        return result;
    }
    if (m_points.size() < 3)
    {
        err_handler(stdutils::io::Severity::ERROR, "Not enough points to triangulate. The output will be empty.");
        return result;
    }
    assert(!m_polylines_indices.empty());
    try
    {
        std::vector<p2t::Point> p2t_points = copy_p2t_vertices(m_points);
        result.vertices = m_points;

        // As per poly2tri documentation:
        // Initialize CDT with a simple polyline (this defines the constrained edges)
        const auto main_polyline = has_main_path ? to_p2t_polyline<I>(p2t_points, m_polylines_indices.front()) : std::vector<p2t::Point*>();
        p2t::CDT cdt(main_polyline);

        // Add holes if necessary (also simple polylines)
        for (unsigned int path_idx = 1; path_idx < m_polylines_indices.size(); path_idx++)
        {
            const auto hole_polyline = to_p2t_polyline<I>(p2t_points, m_polylines_indices.at(path_idx));
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
        std::vector<p2t::Triangle*> p2t_triangles = cdt.GetTriangles();

        p2t::Point* begin_point = &p2t_points[0];
        result.faces.reserve(p2t_triangles.size());
        shapes::Triangles2d<F, I>::face arr;
        for (const auto& triangle : p2t_triangles)
        {
            for (unsigned int i = 0; i < 3; i++)
                arr[i] = std::distance(begin_point, triangle->GetPoint(i));
            result.faces.emplace_back(arr);
        }
    }
    catch(const std::exception& e)
    {
        result.vertices.clear();
        result.faces.clear();
        std::stringstream out;
        out << "Poly2tri exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, out.str());
    }
    assert(is_valid(result));
    return result;
}

} // namespace delaunay
