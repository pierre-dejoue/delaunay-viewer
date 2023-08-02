#include <catch2/catch_test_macros.hpp>

#include <shapes/conversion.h>
#include <shapes/shapes.h>

#include <cassert>
#include <vector>
#include <sstream>
#include <string>
#include <type_traits>

namespace shapes
{
template <typename F>
PointCloud2d<F> test_point_cloud_2d()
{
    PointCloud2d<F> out;
    out.vertices.emplace_back(F{1}, F{2});
    out.vertices.emplace_back(F{3}, F{0});
    out.vertices.emplace_back(F{4}, F{1});
    return out;
}

template <typename F>
PointCloud3d<F> test_point_cloud_3d()
{
    PointCloud3d<F> out;
    out.vertices.emplace_back(F{1}, F{2}, F{1});
    out.vertices.emplace_back(F{3}, F{0}, F{-1});
    out.vertices.emplace_back(F{4}, F{1}, F{-2});
    return out;
}

template <typename F>
PointPath2d<F> test_point_path_2d() { return to_point_path(test_point_cloud_2d<F>(), CLOSED_PATH); }

template <typename F>
PointPath3d<F> test_point_path_3d() { return to_point_path(test_point_cloud_3d<F>(), CLOSED_PATH); }

template <typename F>
CubicBezierPath2d<F> test_cbp_2d()
{
    CubicBezierPath2d<F> out;
    out.closed = false;
    out.vertices.emplace_back(F{1}, F{0});
    out.vertices.emplace_back(F{3}, F{1});
    out.vertices.emplace_back(F{4}, F{-1});
    out.vertices.emplace_back(F{5}, F{0});
    assert(valid_size(out));
    return out;
}

template <typename F>
CubicBezierPath3d<F> test_cbp_3d()
{
    CubicBezierPath3d<F> out;
    out.closed = false;
    out.vertices.emplace_back(F{1}, F{0}, F{1});
    out.vertices.emplace_back(F{3}, F{1}, F{1});
    out.vertices.emplace_back(F{4}, F{-1}, F{1});
    out.vertices.emplace_back(F{5}, F{0}, F{1});
    assert(valid_size(out));
    return out;
}

template <typename F>
Triangles2d<F> test_triangles_2d()
{
    Triangles2d<F> out;
    out.vertices.emplace_back(F{1}, F{2});
    out.vertices.emplace_back(F{3}, F{0});
    out.vertices.emplace_back(F{4}, F{1});
    out.faces.emplace_back(Triangles2d<F>::face {0, 1, 2});
    return out;
}

template <typename F>
Triangles3d<F> test_triangles_3d()
{
    Triangles3d<F> out;
    out.vertices.emplace_back(F{1}, F{2}, F{1});
    out.vertices.emplace_back(F{3}, F{0}, F{-1});
    out.vertices.emplace_back(F{4}, F{1}, F{-2});
    out.faces.emplace_back(Triangles3d<F>::face {0, 1, 2});
    return out;
}

TEST_CASE("Generic functions on AllShapes variant", "[shapes]")
{
    AllShapes<double> s;

    s = test_point_cloud_2d<double>();
    CHECK(get_dimension(s) == 2);
    CHECK(nb_vertices(s) == 3);
    CHECK(nb_edges(s) == 0);
    CHECK(nb_faces(s) == 0);

    s = test_point_cloud_3d<double>();
    CHECK(get_dimension(s) == 3);
    CHECK(nb_vertices(s) == 3);
    CHECK(nb_edges(s) == 0);
    CHECK(nb_faces(s) == 0);

    s = test_point_path_2d<double>();
    CHECK(get_dimension(s) == 2);
    CHECK(nb_vertices(s) == 3);
    CHECK(nb_edges(s) == 3);
    CHECK(nb_faces(s) == 0);

    s = test_point_path_3d<double>();
    CHECK(get_dimension(s) == 3);
    CHECK(nb_vertices(s) == 3);
    CHECK(nb_edges(s) == 3);
    CHECK(nb_faces(s) == 0);

    s = test_cbp_2d<double>();
    CHECK(get_dimension(s) == 2);
    CHECK(nb_vertices(s) == 4);
    CHECK(nb_edges(s) == 1);
    CHECK(nb_faces(s) == 0);

    s = test_cbp_3d<double>();
    CHECK(get_dimension(s) == 3);
    CHECK(nb_vertices(s) == 4);
    CHECK(nb_edges(s) == 1);
    CHECK(nb_faces(s) == 0);

    s = test_triangles_2d<double>();
    CHECK(get_dimension(s) == 2);
    CHECK(nb_vertices(s) == 3);
    CHECK(nb_edges(s) == 3);
    CHECK(nb_faces(s) == 1);

    s = test_triangles_3d<double>();
    CHECK(get_dimension(s) == 3);
    CHECK(nb_vertices(s) == 3);
    CHECK(nb_edges(s) == 3);
    CHECK(nb_faces(s) == 1);
}

}
