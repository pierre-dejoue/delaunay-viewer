// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include "draw_command.h"
#include "drawing_options.h"
#include "renderer.h"

#include <base/canvas.h>
#include <base/color_data.h>
#include <shapes/edge.h>
#include <shapes/point_cloud.h>
#include <shapes/path.h>
#include <shapes/path_algos.h>
#include <shapes/triangle.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <utility>

template <typename F>
void draw_point_cloud(const shapes::PointCloud2d<F>& pc, renderer::DrawList& draw_list, const DrawingOptions& options);

template <typename F>
void draw_point_path(const shapes::PointPath2d<F>& pp, renderer::DrawList& draw_list, const DrawingOptions& options);

template <typename F>
void draw_edge_soup(const shapes::Edges2d<F>& pp, renderer::DrawList& draw_list, const DrawingOptions& options);

template <typename F, typename I>
void draw_triangles(const shapes::Triangles2d<F, I>& tri, renderer::DrawList& draw_list, const DrawingOptions& options);

// NB: draw_cubic_bezier_path() does not exist. CBP are converted to point paths.


//
//
// Implementation
//
//


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
