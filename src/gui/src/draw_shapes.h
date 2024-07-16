// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include "draw_command.h"
#include "drawing_options.h"
#include "renderer.h"

#include <base/canvas.h>
#include <base/color_data.h>
#include <imgui/imgui.h>
#include <shapes/edge.h>
#include <shapes/point_cloud.h>
#include <shapes/path.h>
#include <shapes/path_algos.h>
#include <shapes/triangle.h>

#include <cassert>

/**
 * Shapes drawing:
 *  - with ImGui
 *  - with our own renderer
 */

template <typename F>
void draw_point_cloud(const shapes::PointCloud2d<F>& pc, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options);
template <typename F>
void draw_point_cloud(const shapes::PointCloud2d<F>& pc, renderer::DrawList& draw_list, const DrawingOptions& options);

template <typename F>
void draw_point_path(const shapes::PointPath2d<F>& pp, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options);
template <typename F>
void draw_point_path(const shapes::PointPath2d<F>& pp, renderer::DrawList& draw_list, const DrawingOptions& options);

template <typename F>
void draw_cubic_bezier_path(const shapes::CubicBezierPath2d<F>& cbp, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options);
template <typename F>
void draw_cubic_bezier_path(const shapes::CubicBezierPath2d<F>& cbp, renderer::DrawList& draw_list,  const DrawingOptions& options);

template <typename F>
void draw_edge_soup(const shapes::Edges2d<F>& pp, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options);
template <typename F>
void draw_edge_soup(const shapes::Edges2d<F>& pp, renderer::DrawList& draw_list, const DrawingOptions& options);

template <typename F, typename I>
void draw_triangles(const shapes::Triangles2d<F, I>& tri, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options);
template <typename F, typename I>
void draw_triangles(const shapes::Triangles2d<F, I>& tri, renderer::DrawList& draw_list, const DrawingOptions& options);


//
//
// Implementation
//
//


namespace details {

    // Draw all the vertices of any shape with vertices
    template <typename S, typename F = typename S::scalar>
    void draw_points(const S& s, ImDrawList* draw_list, const Canvas<F>& canvas, ImU32 color, float point_width)
    {
        assert(draw_list);
        if (point_width <= 1.f)
            for (const auto& p: s.vertices)
            {
                const auto cp = canvas.to_screen(p);
                draw_list->AddRectFilled(to_imgui_vec2(cp), ImVec2(cp.x + 1.f, cp.y + 1.f), color);
            }
        else
            for (const auto& p: s.vertices)
            {
                const auto cp = canvas.to_screen(p);
                draw_list->AddCircleFilled(to_imgui_vec2(cp), 0.5f * point_width, color);
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

    inline ImU32 to_compact_color(const ColorData& color)
    {
        const ImColor im_color(color[0], color[1], color[2], color[3]);
        return static_cast<ImU32>(im_color);
    }

} // namespace details

template <typename F>
void draw_point_cloud(const shapes::PointCloud2d<F>& pc, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options)
{
    assert(draw_list);
    if (options.point_options.show && options.vertices.draw)
    {
        const auto color = details::to_compact_color(options.vertices.color);
        details::draw_points(pc, draw_list, canvas, color, options.point_options.size);
    }
}

template <typename F>
void draw_point_cloud(const shapes::PointCloud2d<F>& pc, renderer::DrawList& draw_list, const DrawingOptions& options)
{
    const std::size_t nb_vertices = pc.vertices.size();
    const auto begin_indices_idx = draw_list.m_indices.consumed();
    const auto   end_indices_idx = draw_list.m_indices.consumed() + nb_vertices;
    if (draw_list.m_vertices.is_unlocked())
    {
        assert(draw_list.m_indices.is_unlocked());
        // Vertices
        const auto begin_vertex_idx = draw_list.m_vertices.consumed();
        std::transform(std::cbegin(pc.vertices), std::cend(pc.vertices), std::back_inserter(draw_list.m_vertices.buffer()), [](const shapes::Point2d<F>& p) {
            return renderer::DrawList::VertexData{ static_cast<float>(p.x), static_cast<float>(p.y), 0.f };
        });
        // Indices
        for (std::size_t idx = 0; idx < nb_vertices; idx++)
        {
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + idx));
        }
    }

    // Align the buffer indices
    draw_list.m_vertices.consume(nb_vertices);
    draw_list.m_indices.consume(nb_vertices);

    // Show/no_show is decided by the presence of a DrawCmd (do not modify the vertices and indices buffers)
    if (options.point_options.show && options.vertices.draw)
    {
        auto& draw_call = draw_list.m_draw_calls.emplace_back();
        draw_call.m_range = std::make_pair(begin_indices_idx, end_indices_idx);
        draw_call.m_uniform_color = options.vertices.color;
        draw_call.m_uniform_point_size = std::max(1.f, options.point_options.size);
        draw_call.m_cmd = renderer::DrawCmd::Points;
    }
}

template <typename F>
void draw_point_path(const shapes::PointPath2d<F>& pp, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options)
{
    assert(draw_list);
    const std::size_t nb_vertices = pp.vertices.size();
    if (options.path_options.show && options.edges.draw)
    {
        const auto nb_edges = shapes::nb_edges(pp);
        assert(nb_edges <= nb_vertices);
        const ImU32 color = details::to_compact_color(options.edges.color);
        for (std::size_t idx = 0; idx < nb_edges; idx++)
        {
            const shapes::Point2d<F>& p0 = pp.vertices[idx];
            const shapes::Point2d<F>& p1 = pp.vertices[(idx + 1) % nb_vertices];
            const auto cp0 = canvas.to_screen(p0);
            const auto cp1 = canvas.to_screen(p1);
            details::custom_add_line(draw_list, to_imgui_vec2(cp0), to_imgui_vec2(cp1), color, options.path_options.width);
        }
    }
    if (options.point_options.show && options.vertices.draw)
    {
        const ImU32 color = details::to_compact_color(options.vertices.color);
        details::draw_points(pp, draw_list, canvas, color, options.point_options.size);
    }
}

template <typename F>
void draw_point_path(const shapes::PointPath2d<F>& pp, renderer::DrawList& draw_list, const DrawingOptions& options)
{
    const std::size_t nb_vertices = pp.vertices.size();
    const auto nb_edges = shapes::nb_edges(pp);
    assert(nb_edges <= pp.vertices.size());
    const auto begin_edge_indices_idx         = draw_list.m_indices.consumed();
    const auto   end_edge_indices_idx         = draw_list.m_indices.consumed() + 2 * nb_edges;
    const auto begin_point_indices_idx        = draw_list.m_indices.consumed() + 2 * nb_edges;
    const auto   end_point_indices_idx        = draw_list.m_indices.consumed() + 2 * nb_edges + nb_vertices;

    if (draw_list.m_vertices.is_unlocked())
    {
        assert(draw_list.m_indices.is_unlocked());
        // Vertices
        const auto begin_vertex_idx = draw_list.m_vertices.consumed();
        std::transform(std::cbegin(pp.vertices), std::cend(pp.vertices), std::back_inserter(draw_list.m_vertices.buffer()), [](const shapes::Point2d<F>& p) {
            return renderer::DrawList::VertexData{ static_cast<float>(p.x), static_cast<float>(p.y), 0.f };
        });
        // Indices
        for (std::size_t idx = 0; idx < nb_edges; idx++)
        {
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + idx));
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + ((idx + 1) % nb_vertices)));
        }
        for (std::size_t idx = 0; idx < nb_vertices; idx++)
        {
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + idx));
        }
    }

    // Align the buffer indices
    draw_list.m_vertices.consume(nb_vertices);
    draw_list.m_indices.consume(2 * nb_edges + nb_vertices);

    // Show/no_show is decided by the presence of a DrawCmd (do not modify the vertices and indices buffers)
    if (options.path_options.show && options.edges.draw)
    {
        auto& draw_call = draw_list.m_draw_calls.emplace_back();
        draw_call.m_range = std::make_pair(begin_edge_indices_idx, end_edge_indices_idx);
        draw_call.m_uniform_color = options.edges.color;
        draw_call.m_cmd = renderer::DrawCmd::Lines;
    }
    if (options.point_options.show && options.vertices.draw)
    {
        auto& draw_call = draw_list.m_draw_calls.emplace_back();
        draw_call.m_range = std::make_pair(begin_point_indices_idx, end_point_indices_idx);
        draw_call.m_uniform_color = options.vertices.color;
        draw_call.m_uniform_point_size = std::max(1.f, options.point_options.size);
        draw_call.m_cmd = renderer::DrawCmd::Points;
    }
}

template <typename F>
void draw_cubic_bezier_path(const shapes::CubicBezierPath2d<F>& cbp, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options)
{
    assert(draw_list);
    if (options.path_options.show && options.edges.draw)
    {
        const ImU32 color = details::to_compact_color(options.edges.color);
        const auto nb_segments = shapes::nb_segments(cbp);
        for (std::size_t idx = 0; idx < nb_segments; idx++)
        {
            assert(3 * idx + 2 < cbp.vertices.size());
            const shapes::Point2d<F>& p0 = cbp.vertices[3 * idx];
            const shapes::Point2d<F>& p1 = cbp.vertices[3 * idx + 1];
            const shapes::Point2d<F>& p2 = cbp.vertices[3 * idx + 2];
            const shapes::Point2d<F>& p3 = cbp.vertices[(3 * idx + 3) % cbp.vertices.size()];
            const auto cp0 = canvas.to_screen(p0);
            const auto cp1 = canvas.to_screen(p1);
            const auto cp2 = canvas.to_screen(p2);
            const auto cp3 = canvas.to_screen(p3);
            draw_list->AddBezierCubic(to_imgui_vec2(cp0), to_imgui_vec2(cp1), to_imgui_vec2(cp2), to_imgui_vec2(cp3), color, options.path_options.width);
        }
    }
    if (options.point_options.show && options.vertices.draw)
    {
        const shapes::PointPath2d<F> pp = shapes::extract_endpoints(cbp);
        const ImU32 color = details::to_compact_color(options.vertices.color);
        details::draw_points(pp, draw_list, canvas, color, options.point_options.size);
    }
}

template <typename F>
void draw_cubic_bezier_path(const shapes::CubicBezierPath2d<F>& cbp, renderer::DrawList& draw_list, const DrawingOptions& options)
{
    UNUSED(cbp); UNUSED(draw_list); UNUSED(options);
    // To be implemented
    // For now, the cubic Bezier curves are rendered thanks to ImGui primitives, on top of our rendering of the other shapes.
}

template <typename F>
void draw_edge_soup(const shapes::Edges2d<F>& es, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options)
{
    assert(draw_list);
    if (options.path_options.show && options.edges.draw)
    {
        const ImU32 color = details::to_compact_color(options.edges.color);
        for (const auto& edge : es.indices)
        {
            const std::size_t i = static_cast<std::size_t>(edge[0]);
            const std::size_t j = static_cast<std::size_t>(edge[1]);
            assert(i < es.vertices.size());
            assert(j < es.vertices.size());
            const shapes::Point2d<F>& p0 = es.vertices[i];
            const shapes::Point2d<F>& p1 = es.vertices[j];
            const auto cp0 = canvas.to_screen(p0);
            const auto cp1 = canvas.to_screen(p1);
            details::custom_add_line(draw_list, to_imgui_vec2(cp0), to_imgui_vec2(cp1), color, options.path_options.width);
        }
    }
    if (options.point_options.show && options.vertices.draw)
    {
        const ImU32 color = details::to_compact_color(options.vertices.color);
        details::draw_points(es, draw_list, canvas, color, options.point_options.size);
    }
}

template <typename F>
void draw_edge_soup(const shapes::Edges2d<F>& es, renderer::DrawList& draw_list, const DrawingOptions& options)
{
        const std::size_t nb_vertices = es.vertices.size();
    const std::size_t nb_edges = es.indices.size();
    const auto begin_edge_indices_idx  = draw_list.m_indices.consumed();
    const auto   end_edge_indices_idx  = draw_list.m_indices.consumed() + 2 * nb_edges;
    const auto begin_point_indices_idx = draw_list.m_indices.consumed() + 2 * nb_edges;
    const auto   end_point_indices_idx = draw_list.m_indices.consumed() + 2 * nb_edges + nb_vertices;

    if (draw_list.m_vertices.is_unlocked())
    {
        assert(draw_list.m_indices.is_unlocked());
        // Vertices
        const auto begin_vertex_idx = draw_list.m_vertices.consumed();
        std::transform(std::cbegin(es.vertices), std::cend(es.vertices), std::back_inserter(draw_list.m_vertices.buffer()), [](const shapes::Point2d<F>& p) {
            return renderer::DrawList::VertexData{ static_cast<float>(p.x), static_cast<float>(p.y), 0.f };
        });
        // Indices
        for (const auto& edge : es.indices)
        {
            const std::size_t i = static_cast<std::size_t>(edge[0]);
            const std::size_t j = static_cast<std::size_t>(edge[1]);
            assert(i < nb_vertices);
            assert(j < nb_vertices);
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + i));
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + j));
        }
        for (std::size_t i = 0; i < nb_vertices; i++)
        {
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + i));
        }
    }

    // Align the buffer indices
    draw_list.m_vertices.consume(nb_vertices);
    draw_list.m_indices.consume(2 * nb_edges + nb_vertices);

    // Show/no_show is decided by the presence of a DrawCmd (do not modify the vertices and indices buffers)
    if (options.path_options.show && options.edges.draw)
    {
        auto& draw_call = draw_list.m_draw_calls.emplace_back();
        draw_call.m_range = std::make_pair(begin_edge_indices_idx, end_edge_indices_idx);
        draw_call.m_uniform_color = options.edges.color;
        draw_call.m_cmd = renderer::DrawCmd::Lines;
    }
    if (options.point_options.show && options.vertices.draw)
    {
        auto& draw_call = draw_list.m_draw_calls.emplace_back();
        draw_call.m_range = std::make_pair(begin_point_indices_idx, end_point_indices_idx);
        draw_call.m_uniform_color = options.vertices.color;
        draw_call.m_uniform_point_size = std::max(1.f, options.point_options.size);
        draw_call.m_cmd = renderer::DrawCmd::Points;
    }
}

template <typename F, typename I>
void draw_triangles(const shapes::Triangles2d<F, I>& tri, ImDrawList* draw_list, const Canvas<F>& canvas, const DrawingOptions& options)
{
    assert(draw_list);
    if (options.surface_options.show && options.faces.draw)
    {
        const ImU32 color = details::to_compact_color(options.faces.color);
        for (const auto& face : tri.faces)
        {
            const shapes::Point2d<F>& p0 = tri.vertices[face[0]];
            const shapes::Point2d<F>& p1 = tri.vertices[face[1]];
            const shapes::Point2d<F>& p2 = tri.vertices[face[2]];
            const auto cp0 = canvas.to_screen(p0);
            const auto cp1 = canvas.to_screen(p1);
            const auto cp2 = canvas.to_screen(p2);
            draw_list->AddTriangleFilled(to_imgui_vec2(cp0), to_imgui_vec2(cp1), to_imgui_vec2(cp2), color);
        }
    }
    if (options.path_options.show && options.edges.draw)
    {
        const ImU32 color = details::to_compact_color(options.edges.color);
        const float thickness =  options.path_options.width;
        for (const auto& face : tri.faces)
        {
            const shapes::Point2d<F>& p0 = tri.vertices[face[0]];
            const shapes::Point2d<F>& p1 = tri.vertices[face[1]];
            const shapes::Point2d<F>& p2 = tri.vertices[face[2]];
            const auto cp0 = to_imgui_vec2(canvas.to_screen(p0));
            const auto cp1 = to_imgui_vec2(canvas.to_screen(p1));
            const auto cp2 = to_imgui_vec2(canvas.to_screen(p2));
            details::custom_add_line(draw_list, cp0, cp1, color, thickness);
            details::custom_add_line(draw_list, cp1, cp2, color, thickness);
            details::custom_add_line(draw_list, cp2, cp0, color, thickness);
        }
    }
    if (options.point_options.show && options.vertices.draw)
    {
        const ImU32 color = details::to_compact_color(options.vertices.color);
        details::draw_points(tri, draw_list, canvas, color, options.point_options.size);
    }
}

template <typename F, typename I>
void draw_triangles(const shapes::Triangles2d<F, I>& tri, renderer::DrawList& draw_list, const DrawingOptions& options)
{
    const std::size_t nb_vertices = tri.vertices.size();
    const std::size_t nb_faces = tri.faces.size();
    const auto begin_face_indices_idx  = draw_list.m_indices.consumed();
    const auto   end_face_indices_idx  = draw_list.m_indices.consumed() + 3 * nb_faces;
    const auto begin_edge_indices_idx  = draw_list.m_indices.consumed() + 3 * nb_faces;
    const auto   end_edge_indices_idx  = draw_list.m_indices.consumed() + 9 * nb_faces;
    const auto begin_point_indices_idx = draw_list.m_indices.consumed() + 9 * nb_faces;
    const auto   end_point_indices_idx = draw_list.m_indices.consumed() + 9 * nb_faces + nb_vertices;

    if (draw_list.m_vertices.is_unlocked())
    {
        assert(draw_list.m_indices.is_unlocked());
        // Vertices
        const auto begin_vertex_idx = draw_list.m_vertices.consumed();
        std::transform(std::cbegin(tri.vertices), std::cend(tri.vertices), std::back_inserter(draw_list.m_vertices.buffer()), [](const shapes::Point2d<F>& p) {
            return renderer::DrawList::VertexData{ static_cast<float>(p.x), static_cast<float>(p.y), 0.f };
        });
        // Indices
        for (const auto& face : tri.faces)
        {
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + face[0]));
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + face[1]));
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + face[2]));
        }
        for (const auto& face : tri.faces)
        {
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + face[0]));
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + face[1]));
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + face[1]));
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + face[2]));
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + face[2]));
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + face[0]));
        }
        for (std::size_t idx = 0; idx < nb_vertices; idx++)
        {
            draw_list.m_indices.buffer().emplace_back(static_cast<renderer::DrawList::HWindex>(begin_vertex_idx + idx));
        }
    }

    // Align the buffer indices
    draw_list.m_vertices.consume(nb_vertices);
    draw_list.m_indices.consume(9 * nb_faces + nb_vertices);

    // Show/no_show is decided by the presence of a DrawCmd (do not modify the vertices and indices buffers)
    if (options.surface_options.show && options.faces.draw)
    {
        auto& draw_call = draw_list.m_draw_calls.emplace_back();
        draw_call.m_range = std::make_pair(begin_face_indices_idx, end_face_indices_idx);
        draw_call.m_uniform_color = options.faces.color;
        draw_call.m_cmd = renderer::DrawCmd::Triangles;
    }
    if (options.path_options.show && options.edges.draw)
    {
        auto& draw_call = draw_list.m_draw_calls.emplace_back();
        draw_call.m_range = std::make_pair(begin_edge_indices_idx, end_edge_indices_idx);
        draw_call.m_uniform_color = options.edges.color;
        draw_call.m_cmd = renderer::DrawCmd::Lines;
    }
    if (options.point_options.show && options.vertices.draw)
    {
        auto& draw_call = draw_list.m_draw_calls.emplace_back();
        draw_call.m_range = std::make_pair(begin_point_indices_idx, end_point_indices_idx);
        draw_call.m_uniform_color = options.vertices.color;
        draw_call.m_uniform_point_size = std::max(1.f, options.point_options.size);
        draw_call.m_cmd = renderer::DrawCmd::Points;
    }
}

