#pragma once

#include <shapes/path.h>
#include <shapes/point_cloud.h>
#include <shapes/triangle.h>
#include <stdutils/visit.h>

#include <cassert>
#include <string_view>
#include <type_traits>
#include <variant>

namespace shapes
{

// NB: The first type in the list is default constructible therefore the variant is as well
template <typename F>
using AllShapes = std::variant<
    shapes::PointCloud2d<F>,
    shapes::PointCloud3d<F>,
    shapes::PointPath2d<F>,
    shapes::PointPath3d<F>,
    shapes::CubicBezierPath2d<F>,
    shapes::CubicBezierPath3d<F>,
    shapes::Triangles2d<F>,
    shapes::Triangles3d<F>
>;

template <typename F>
std::string_view get_type_str(const AllShapes<F>& shape)
{
    std::string_view result = "unknown";
    std::visit(stdutils::Overloaded {
        [&result](const shapes::PointCloud2d<F>&)      { result = "shapes::PointCloud2d<F>"; },
        [&result](const shapes::PointCloud3d<F>&)      { result = "shapes::PointCloud3d<F>"; },
        [&result](const shapes::PointPath2d<F>&)       { result = "shapes::PointPath2d<F>"; },
        [&result](const shapes::PointPath3d<F>&)       { result = "shapes::PointPath3d<F>"; },
        [&result](const shapes::CubicBezierPath2d<F>&) { result = "shapes::CubicBezierPath2d<F>"; },
        [&result](const shapes::CubicBezierPath3d<F>&) { result = "shapes::CubicBezierPath3d<F>"; },
        [&result](const shapes::Triangles2d<F>&)       { result = "shapes::Triangles2d<F>"; },
        [&result](const shapes::Triangles3d<F>&)       { result = "shapes::Triangles3d<F>"; },
        [](const auto&) { assert(0); }
    }, shape);
    return result;
}

template <typename F>
int get_dimension(const AllShapes<F>& shape)
{
    return std::visit([](const auto& s) { using T = std::decay_t<decltype(s)>; return T::dim; }, shape);
}

template <typename F>
std::size_t nb_vertices(const AllShapes<F>& shape)
{
    return std::visit([](const auto& s) { return s.vertices.size(); }, shape);
}

template <typename F>
std::size_t nb_edges(const AllShapes<F>& shape)
{
    std::size_t result = 0;
    std::visit(stdutils::Overloaded {
        [](const shapes::PointCloud2d<F>&)               { /* Do nothing */ },
        [](const shapes::PointCloud3d<F>&)               { /* Do nothing */ },
        [&result](const shapes::PointPath2d<F>& s)       { result = nb_edges(s); },
        [&result](const shapes::PointPath3d<F>& s)       { result = nb_edges(s); },
        [&result](const shapes::CubicBezierPath2d<F>& s) { result = nb_edges(s); },
        [&result](const shapes::CubicBezierPath3d<F>& s) { result = nb_edges(s); },
        [&result](const shapes::Triangles2d<F>& s)       { result = nb_edges(s); },
        [&result](const shapes::Triangles3d<F>& s)       { result = nb_edges(s); },
        [](const auto&) { assert(0); }
    }, shape);
    return result;
}

template <typename F>
std::size_t nb_faces(const AllShapes<F>& shape)
{
    std::size_t result = 0;
    std::visit(stdutils::Overloaded {
        [](const shapes::PointCloud2d<F>&)              { /* Do nothing */ },
        [](const shapes::PointCloud3d<F>&)              { /* Do nothing */ },
        [](const shapes::PointPath2d<F>&)               { /* Do nothing */ },
        [](const shapes::PointPath3d<F>&)               { /* Do nothing */ },
        [](const shapes::CubicBezierPath2d<F>&)         { /* Do nothing */ },
        [](const shapes::CubicBezierPath3d<F>&)         { /* Do nothing */ },
        [&result](const shapes::Triangles2d<F>& s)      { result = s.faces.size(); },
        [&result](const shapes::Triangles3d<F>& s)      { result = s.faces.size(); },
        [](const auto&) { assert(0); }
    }, shape);
    return result;
}

template <typename F>
bool is_point_cloud(const AllShapes<F>& shape)
{
    return std::visit(stdutils::Overloaded {
        [](const shapes::PointCloud2d<F>&)          { return true; },
        [](const shapes::PointCloud3d<F>&)          { return true; },
        [](const auto&)                             { return false; }
    }, shape);
}

template <typename F>
bool is_point_path(const AllShapes<F>& shape)
{
    return std::visit(stdutils::Overloaded {
        [](const shapes::PointPath2d<F>&)           { return true; },
        [](const shapes::PointPath3d<F>&)           { return true; },
        [](const auto&)                             { return false; }
    }, shape);
}

template <typename F>
bool is_bezier_path(const AllShapes<F>& shape)
{
    return std::visit(stdutils::Overloaded {
        [](const shapes::CubicBezierPath2d<F>&)     { return true; },
        [](const shapes::CubicBezierPath3d<F>&)     { return true; },
        [](const auto&)                             { return false; }
    }, shape);
}

template <typename F>
bool has_edges(const AllShapes<F>& shape)
{
    return std::visit(stdutils::Overloaded {
        [](const shapes::PointCloud2d<F>&)          { return false; },
        [](const shapes::PointCloud3d<F>&)          { return false; },
        [](const auto&)                             { return true; }
    }, shape);
}

template <typename F>
bool has_faces(const AllShapes<F>& shape)
{
    return std::visit(stdutils::Overloaded {
        [](const shapes::Triangles2d<F>&)           { return true; },
        [](const shapes::Triangles3d<F>&)           { return true; },
        [](const auto&)                             { return false; }
    }, shape);
}

} // namespace shapes
