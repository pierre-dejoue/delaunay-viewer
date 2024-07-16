// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "renderer_helpers.h"

#include "draw_shapes.h"

template <typename F>
void update_opengl_draw_list(renderer::DrawList& draw_list, const DrawCommands<F>& draw_commands, bool update_buffers, const DrawingOptions& options)
{
    if (update_buffers || draw_list.buffer_version() == 0) { draw_list.clear_all(); } else { draw_list.clear_draw_calls(); }
    assert(draw_list.m_vertices.consumed() == 0);
    assert(draw_list.m_indices.consumed() == 0);

    DrawingOptions local_options = options;
    for (const auto& draw_command : draw_commands)
    {
        assert(draw_command.shape != nullptr);
        local_options.vertices = draw_command.vertices;
        local_options.edges = draw_command.edges;
        local_options.faces = draw_command.faces;
        std::visit(stdutils::Overloaded {
            [&draw_list, &local_options](const shapes::PointCloud2d<F>& pc)       { draw_point_cloud(pc, draw_list, local_options); },
            [&draw_list, &local_options](const shapes::PointPath2d<F>& pp)        { draw_point_path(pp, draw_list, local_options); },
            [&draw_list, &local_options](const shapes::CubicBezierPath2d<F>& cbp) { draw_cubic_bezier_path(cbp, draw_list, local_options); },
            [&draw_list, &local_options](const shapes::Edges2d<F>& es)            { draw_edge_soup(es, draw_list, local_options); },
            [&draw_list, &local_options](const shapes::Triangles2d<F>& tri)       { draw_triangles(tri, draw_list, local_options); },
            [](const auto&) { assert(0); }
        }, *draw_command.shape);
    }

    // Ensure the buffers are locked
    assert(draw_list.m_vertices.index_is_aligned());
    assert(draw_list.m_indices.index_is_aligned());
    if (draw_list.m_vertices.is_unlocked())
    {
        assert(draw_list.m_indices.is_unlocked());
        draw_list.m_vertices.lock();
        draw_list.m_indices.lock();
    }
}

// Explicit template instantiations
template void update_opengl_draw_list<double>(renderer::DrawList&, const DrawCommands<double>&, bool, const DrawingOptions&);
