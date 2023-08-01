#pragma once

#include <shapes/path_algos.h>
#include <shapes/shapes.h>

namespace shapes
{

template <typename F>
AllShapes<F> trivial_sampling(const AllShapes<F>& shape)
{
    AllShapes<F> result;
    std::visit(stdutils::Overloaded {
        [&result](const shapes::PointCloud2d<F>& pc)        { result = pc; },
        [&result](const shapes::PointCloud3d<F>& pc)        { result = pc; },
        [&result](const shapes::PointPath2d<F>& pp)         { result = pp; },
        [&result](const shapes::PointPath3d<F>& pp)         { result = pp; },
        [&result](const shapes::CubicBezierPath2d<F>& cbp)  { result = extract_endpoints(cbp); },
        [&result](const shapes::CubicBezierPath3d<F>& cbp)  { result = extract_endpoints(cbp); },
        [&result](const shapes::Triangles2d<F>&)            { /* TBD */ },
        [&result](const shapes::Triangles3d<F>&)            { /* TBD */ },
        [](const auto&) { assert(0); }
    }, shape);
    return result;
}


} // namespace shapes
