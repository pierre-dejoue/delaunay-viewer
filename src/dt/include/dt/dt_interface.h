#pragma once

#include <shapes/path.h>
#include <shapes/point_cloud.h>
#include <shapes/triangle.h>
#include <stdutils/io.h>

#include <cstdint>
#include <memory>


namespace delaunay
{

template <typename F, typename I>
class Interface
{
public:
    Interface() = default;
    virtual ~Interface() = default;

    virtual void add_path(const shapes::PointPath2d<F>& pp, const stdutils::io::ErrorHandler& err_handler) = 0;
    virtual void add_hole(const shapes::PointPath2d<F>& pp, const stdutils::io::ErrorHandler& err_handler) = 0;
    virtual void add_steiner(const shapes::PointCloud2d<F>& pc, const stdutils::io::ErrorHandler& err_handler) = 0;

    virtual shapes::Triangles2d<F, I> triangulate(const stdutils::io::ErrorHandler& err_handler) const = 0;
};

} // namespace delaunay
