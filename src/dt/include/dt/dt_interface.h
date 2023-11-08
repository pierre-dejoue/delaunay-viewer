// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/path.h>
#include <shapes/point_cloud.h>
#include <shapes/triangle.h>
#include <stdutils/io.h>

#include <cstdint>
#include <memory>


namespace delaunay
{

enum class TriangulationPolicy
{
    PointCloud,
    CDT,
};

template <typename F, typename I>
class Interface
{
public:
    using scalar = F;
    using index = I;

    Interface(const stdutils::io::ErrorHandler* err_handler);
    virtual ~Interface() = default;

    virtual void add_path(const shapes::PointPath2d<F>& pp) = 0;
    virtual void add_hole(const shapes::PointPath2d<F>& pp) = 0;
    virtual void add_steiner(const shapes::PointCloud2d<F>& pc) = 0;

    shapes::Triangles2d<F, I> triangulate(TriangulationPolicy policy) const noexcept;

protected:
    virtual void triangulate_impl(TriangulationPolicy policy, shapes::Triangles2d<F, I>& result) const = 0;

    stdutils::io::ErrorHandler m_err_handler;
};


//
// Implementation
//

template <typename F, typename I>
Interface<F, I>::Interface(const stdutils::io::ErrorHandler* err_handler)
    : m_err_handler()
{
    if (err_handler) { m_err_handler = *err_handler; }
}

template <typename F, typename I>
shapes::Triangles2d<F, I> Interface<F, I>::triangulate(TriangulationPolicy policy) const noexcept
{
    shapes::Triangles2d<F, I> result;
    try
    {
        triangulate_impl(policy, result);
    }
    catch(const std::exception& e)
    {
        result.vertices.clear();
        result.faces.clear();
        if (m_err_handler) { m_err_handler(stdutils::io::Severity::EXCPT, e.what()); }
    }
    assert(is_valid(result));
    return result;
}


} // namespace delaunay
