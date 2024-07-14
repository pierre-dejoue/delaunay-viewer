// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "imgui_draw_list.h"

#include "draw_shapes.h"

#include <imgui/imgui.h>
#include <stdutils/visit.h>

#include <cassert>
#include <variant>

template <typename F>
void update_imgui_draw_list(ImDrawList& draw_list, const DrawCommands<F>& draw_commands, const Canvas<F>& canvas, const DrawingOptions& options)
{
    DrawingOptions local_options = options;
    for (const auto& draw_command : draw_commands)
    {
        assert(draw_command.shape);
        local_options.vertices = draw_command.vertices;
        local_options.edges = draw_command.edges;
        local_options.faces = draw_command.faces;
        std::visit(stdutils::Overloaded {
            [&draw_list, &canvas, &local_options](const shapes::PointCloud2d<F>& pc) { draw_point_cloud<F>(pc, &draw_list, canvas, local_options); },
            [&draw_list, &canvas, &local_options](const shapes::PointPath2d<F>& pp) { draw_point_path<F>(pp, &draw_list, canvas, local_options); },
            [&draw_list, &canvas, &local_options](const shapes::CubicBezierPath2d<F>& cbp) { draw_cubic_bezier_path<F>(cbp, &draw_list, canvas, local_options); },
            [&draw_list, &canvas, &local_options](const shapes::Edges2d<F>& es) { draw_edge_soup<F>(es, &draw_list, canvas, local_options); },
            [&draw_list, &canvas, &local_options](const shapes::Triangles2d<F>& tri) { draw_triangles<F>(tri, &draw_list, canvas, local_options); },
            [](const auto&) { assert(0); }
        }, *draw_command.shape);
    }
}

// Explicit template instantiations
template void update_imgui_draw_list<double>(ImDrawList&, const DrawCommands<double>&, const Canvas<double>&, const DrawingOptions&);
