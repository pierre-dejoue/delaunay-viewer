// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "renderer_helpers.h"

#include "draw_shapes.h"

#include <shapes/sampling.h>

#include <cassert>
#include <cmath>
#include <algorithm>
#include <variant>

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
            [&draw_list, &local_options](const shapes::PointCloud2d<F>& pc)     { draw_point_cloud(pc, draw_list, local_options); },
            [&draw_list, &local_options](const shapes::PointPath2d<F>& pp)      { draw_point_path(pp, draw_list, local_options); },
            [&draw_list, &local_options](const shapes::Edges2d<F>& es)          { draw_edge_soup(es, draw_list, local_options); },
            [&draw_list, &local_options](const shapes::Triangles2d<F>& tri)     { draw_triangles(tri, draw_list, local_options); },
            []                          (const shapes::CubicBezierPath2d<F>&)   { assert(0); /* CBP should be converted to point paths first */ },
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

template <typename F>
struct CBPSegmentation<F>::Impl
{
    static constexpr float casteljau_length_resolution_in_screen_space = 1.5f;
    static constexpr F resolution_relative_delta_threshold = static_cast<F>(1.0E-2);

    Impl();

    const DrawCommands<F>& convert_cbps(const DrawCommands<F>& draw_commands, const Canvas<float>& viewport_canvas, bool geometry_has_changed, bool& new_segmentation);

    F last_segmentation_resolution;
    shapes::CasteljauSamplingCubicBezier2d<F> casteljau_sampler;
    std::vector<shapes::AllShapes<F>> cbps_endpoints;
    std::vector<shapes::AllShapes<F>> cbps_contour_segmentation;
    DrawCommands<F> result_draw_commands;
};

template <typename F>
CBPSegmentation<F>::Impl::Impl()
    : last_segmentation_resolution{1}
    , casteljau_sampler()
    , cbps_endpoints()
    , cbps_contour_segmentation()
    , result_draw_commands()
{ }

template <typename F>
const DrawCommands<F>& CBPSegmentation<F>::Impl::convert_cbps(const DrawCommands<F>& draw_commands, const Canvas<float>& viewport_canvas, bool geometry_has_changed, bool& new_segmentation)
{
    const F resolution = static_cast<F>(viewport_canvas.to_world(casteljau_length_resolution_in_screen_space));
    assert(resolution > 0);

    const auto nb_cbps = static_cast<std::size_t>(std::count_if(draw_commands.cbegin(), draw_commands.cend(), [](const auto& draw_cmd) { return shapes::is_bezier_path(*draw_cmd.shape); }));

    const F resolution_relative_delta = std::abs(resolution - last_segmentation_resolution) / last_segmentation_resolution;

    new_segmentation =
        geometry_has_changed ||
        nb_cbps != cbps_contour_segmentation.size() ||
        resolution_relative_delta > resolution_relative_delta_threshold;

    if (new_segmentation)
    {
        // Trigger a new segmentation of all CBPs
        last_segmentation_resolution = resolution;
        cbps_endpoints.clear();
        cbps_contour_segmentation.clear();
        cbps_endpoints.reserve(nb_cbps);            // Essential to prevent reallocation and therefore shape pointer invalidation
        cbps_contour_segmentation.reserve(nb_cbps);
    }

    result_draw_commands.clear();
    result_draw_commands.reserve(draw_commands.size() + nb_cbps);       // Each CBP command spawns an additional draw command for the endpoint vertices
    std::size_t cbp_idx = 0;
    for (const auto& draw_command : draw_commands)
    {
        assert(draw_command.shape != nullptr);
        auto& cpy_draw_cmd = result_draw_commands.emplace_back(draw_command);
        std::visit(stdutils::Overloaded {
            [this, resolution, new_segmentation, &cpy_draw_cmd, &cbp_idx](const shapes::CubicBezierPath2d<F>& cbp) {
                if (new_segmentation)
                {
                    cbps_contour_segmentation.emplace_back(casteljau_sampler.sample(cbp, resolution));
                    cbps_endpoints.emplace_back(shapes::extract_endpoints(cbp));
                }
                // Contour draw command
                assert(cbp_idx < cbps_contour_segmentation.size());
                cpy_draw_cmd.shape = &cbps_contour_segmentation[cbp_idx];
                const bool backup_vertices_draw = cpy_draw_cmd.vertices.draw;
                cpy_draw_cmd.vertices.draw = false;
                // Endpoints draw command
                assert(cbp_idx < cbps_endpoints.size());
                auto& endpoints_draw_cmd = result_draw_commands.emplace_back(cbps_endpoints[cbp_idx]);
                endpoints_draw_cmd.vertices = cpy_draw_cmd.vertices;
                endpoints_draw_cmd.vertices.draw = backup_vertices_draw;
                endpoints_draw_cmd.edges.draw = false;
                cbp_idx++;
            },
            [](const auto&) { /* For non-CBP shapes, leave the copy of the draw command as it is */ }
        }, *draw_command.shape);
    }
    assert(cbp_idx == nb_cbps);
    assert(cbps_endpoints.size() == nb_cbps);
    assert(cbps_contour_segmentation.size() == nb_cbps);

    assert(result_draw_commands.size() == draw_commands.size() + nb_cbps);
    return result_draw_commands;
}

template <typename F>
CBPSegmentation<F>::CBPSegmentation()
    : p_impl(std::make_unique<Impl>())
{ }

template <typename F>
CBPSegmentation<F>::~CBPSegmentation() = default;

template <typename F>
void CBPSegmentation<F>::clear_all()
{
    p_impl = std::make_unique<Impl>();
}

template <typename F>
const DrawCommands<F>& CBPSegmentation<F>::convert_cbps(const DrawCommands<F>& draw_commands, const Canvas<float>& viewport_canvas, bool geometry_has_changed, bool& new_segmentation)
{
    return p_impl->convert_cbps(draw_commands, viewport_canvas, geometry_has_changed, new_segmentation);
}

// Explicit template instantiations
template void update_opengl_draw_list<double>(renderer::DrawList&, const DrawCommands<double>&, bool, const DrawingOptions&);
template class CBPSegmentation<double>;
