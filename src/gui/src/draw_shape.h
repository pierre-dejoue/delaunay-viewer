#pragma once

#include "canvas.h"
#include "settings.h"

#include <shapes/point_cloud.h>
#include <shapes/path.h>
#include <shapes/path_algos.h>
#include <shapes/triangle.h>

#include <imgui.h>

#include <cassert>


void draw_canvas_background(ImDrawList* draw_list, ImVec2 tl_corner, ImVec2 br_corner);
void draw_canvas_foreground(ImDrawList* draw_list, ImVec2 tl_corner, ImVec2 br_corner);

struct DrawingOptions
{
    Settings::Point point_settings;
    Settings::Path path_settings;
    Settings::Surface surface_settings;
    bool highlight = false;
    bool constraint_edges = false;
};

ImU32 get_vertex_color(const DrawingOptions& options);
ImU32 get_edge_color(const DrawingOptions& options);
ImU32 get_face_color(const DrawingOptions& options);

template <typename F>
void draw_point_cloud(const shapes::PointCloud2d<F>& pc, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options);

template <typename F>
void draw_point_path(const shapes::PointPath2d<F>& pp, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options);

template <typename F>
void draw_cubic_bezier_path(const shapes::CubicBezierPath2d<F>& cbp, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options);

template <typename F, typename I>
void draw_triangles(const shapes::Triangles2d<F, I>& tri, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options);

//
// Implementations
//

namespace details
{
    // Draw all the vertices of any shape with vertices
    template <typename S, typename F = typename S::scalar>
    void draw_points(const S& s, ImDrawList* draw_list, const Canvas<F>& canvas, ImU32 color, float point_width)
    {
        assert(draw_list);
        if (point_width <= 1.f)
            for (const auto& p: s.vertices)
            {
                const ImVec2 cp = canvas.to_screen(p);
                draw_list->AddRectFilled(cp, ImVec2(cp.x + 1.f, cp.y + 1.f), color);
            }
        else
            for (const auto& p: s.vertices)
            {
                const ImVec2 cp = canvas.to_screen(p);
                draw_list->AddCircleFilled(cp, 0.5f * point_width, color);
            }
    }

    // Same as ImDrawList::AddLine without the +0.5f coordinate shift
    inline void custom_add_line(ImDrawList* draw_list, const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;
        draw_list->PathLineTo(p1);
        draw_list->PathLineTo(p2);
        draw_list->PathStroke(col, 0, thickness);
    }
}

template <typename F>
void draw_point_cloud(const shapes::PointCloud2d<F>& pc, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options)
{
    assert(draw_list);
    if (options.point_settings.show)
    {
        const auto color = get_vertex_color(options);
        details::draw_points(pc, draw_list, canvas, color, options.point_settings.size);
    }
}

template <typename F>
void draw_point_path(const shapes::PointPath2d<F>& pp, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options)
{
    assert(draw_list);
    if (options.path_settings.show)
    {
        assert(shapes::nb_edges(pp) <= pp.vertices.size());
        const ImU32 color = get_edge_color(options);
        for (std::size_t idx = 0; idx < shapes::nb_edges(pp); idx++)
        {
            const shapes::Point2d<F>& p0 = pp.vertices[idx];
            const shapes::Point2d<F>& p1 = pp.vertices[(idx + 1) % pp.vertices.size()];
            const ImVec2 cp0 = canvas.to_screen(p0);
            const ImVec2 cp1 = canvas.to_screen(p1);
            details::custom_add_line(draw_list, cp0, cp1, color, options.path_settings.width);
        }
    }
    if (options.point_settings.show)
    {
        const ImU32 color = get_vertex_color(options);
        details::draw_points(pp, draw_list, canvas, color, options.point_settings.size);
    }
}

template <typename F>
void draw_cubic_bezier_path(const shapes::CubicBezierPath2d<F>& cbp, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options)
{
    assert(draw_list);
    if (options.path_settings.show)
    {
        const ImU32 color = get_edge_color(options);
        for (std::size_t idx = 0; idx < shapes::nb_edges(cbp); idx++)
        {
            assert(3 * idx + 2 < cbp.vertices.size());
            const shapes::Point2d<F>& p0 = cbp.vertices[3 * idx];
            const shapes::Point2d<F>& p1 = cbp.vertices[3 * idx + 1];
            const shapes::Point2d<F>& p2 = cbp.vertices[3 * idx + 2];
            const shapes::Point2d<F>& p3 = cbp.vertices[(3 * idx + 3) % cbp.vertices.size()];
            const ImVec2 cp0 = canvas.to_screen(p0);
            const ImVec2 cp1 = canvas.to_screen(p1);
            const ImVec2 cp2 = canvas.to_screen(p2);
            const ImVec2 cp3 = canvas.to_screen(p3);
            draw_list->AddBezierCubic(cp0, cp1, cp2, cp3, color, options.path_settings.width);
        }
    }
    if (options.point_settings.show)
    {
        const shapes::PointPath2d<F> pp = shapes::extract_endpoints(cbp);
        const ImU32 color = get_vertex_color(options);
        details::draw_points(pp, draw_list, canvas, color, options.point_settings.size);
    }
}

template <typename F, typename I>
void draw_triangles(const shapes::Triangles2d<F, I>& tri, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options)
{
    assert(draw_list);
    if (options.surface_settings.show)
    {
        const ImU32 color = get_face_color(options);
        for (const auto& face : tri.faces)
        {
            const shapes::Point2d<F>& p0 = tri.vertices[face[0]];
            const shapes::Point2d<F>& p1 = tri.vertices[face[1]];
            const shapes::Point2d<F>& p2 = tri.vertices[face[2]];
            const ImVec2 cp0 = canvas.to_screen(p0);
            const ImVec2 cp1 = canvas.to_screen(p1);
            const ImVec2 cp2 = canvas.to_screen(p2);
            draw_list->AddTriangleFilled(cp0, cp1, cp2, color);
        }
    }
    if (options.path_settings.show)
    {
        const ImU32 color = get_edge_color(options);
        const float thickness =  options.path_settings.width;
        for (const auto& face : tri.faces)
        {
            const shapes::Point2d<F>& p0 = tri.vertices[face[0]];
            const shapes::Point2d<F>& p1 = tri.vertices[face[1]];
            const shapes::Point2d<F>& p2 = tri.vertices[face[2]];
            const ImVec2 cp0 = canvas.to_screen(p0);
            const ImVec2 cp1 = canvas.to_screen(p1);
            const ImVec2 cp2 = canvas.to_screen(p2);
            details::custom_add_line(draw_list, cp0, cp1, color, thickness);
            details::custom_add_line(draw_list, cp1, cp2, color, thickness);
            details::custom_add_line(draw_list, cp2, cp0, color, thickness);
        }
    }
    if (options.point_settings.show)
    {
        const ImU32 color = get_vertex_color(options);
        details::draw_points(tri, draw_list, canvas, color, options.point_settings.size);
    }
}
