#include "shape_control_window.h"

#include "draw_shape.h"
#include "settings.h"

#include <dt/dt_interface.h>
#include <shapes/bounding_box_algos.h>
#include <shapes/sampling.h>
#include <stdutils/chrono.h>
#include <stdutils/io.h>
#include <stdutils/macros.h>
#include <stdutils/visit.h>

#include <imgui_wrap.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <variant>


namespace
{
    const char* INPUT_TAB_NAME = "Input";

    ShapeDrawWindow::DrawCommand to_draw_command(const ShapeWindow::ShapeControl& shape_control)
    {
        ShapeDrawWindow::DrawCommand result(shape_control.shape);
        result.highlight = shape_control.highlight;
        result.constraint_edges = shape_control.constraint_edges;
        return result;
    }

    template <typename F>
    void to_opengl_draw_commands(const ShapeWindow::ShapeControl& shape_control, renderer::DrawList& draw_list, DrawingOptions& options)
    {
        options.highlight = shape_control.highlight;
        // options.constraint_edges is set by the caller
        std::visit(stdutils::Overloaded {
            [&draw_list, &options](const shapes::PointCloud2d<F>& pc) { draw_point_cloud(pc, draw_list, options); },
            [&draw_list, &options](const shapes::PointPath2d<F>& pp) { draw_point_path(pp, draw_list, options); },
            [&draw_list, &options](const shapes::CubicBezierPath2d<F>& cbp) { draw_cubic_bezier_path(cbp, draw_list, options); },
            [&draw_list, &options](const shapes::Triangles2d<F>& tri) { draw_triangles(tri, draw_list, options); },
            [](const auto&) { assert(0); }
        }, shape_control.shape);
    }
}

ShapeWindow::ShapeControl::ShapeControl(shapes::AllShapes<scalar>&& shape)
    : nb_vertices(shapes::nb_vertices(shape))
    , nb_edges(shapes::nb_edges(shape))
    , nb_faces(shapes::nb_faces(shape))
    , active(true)
    , force_inactive(false)
    , highlight(false)
    , constraint_edges(false)
    , latest_computation_time_ms(0.f)
    , shape(std::move(shape))
    , sampled_shape(nullptr)
{
}

void ShapeWindow::ShapeControl::update(shapes::AllShapes<scalar>&& rep_shape)
{
    shape = std::move(rep_shape);
    nb_vertices = shapes::nb_vertices(shape);
    nb_edges = shapes::nb_edges(shape);
    nb_faces = shapes::nb_faces(shape);
    // 'active', 'hightlight' and 'constraint_edges' remain as-is
}

ShapeWindow::ShapeWindow(
        std::vector<shapes::AllShapes<scalar>>&& shapes,
        std::string_view name,
        renderer::DrawList& draw_list)
    : m_input_shape_controls()
    , m_geometry_has_changed(true)  // True to trigger the first triangulation
    , m_sampled_shape_controls()
    , m_renderer_draw_list(draw_list)
    , m_title(std::string(name) + " Controls")
    , m_draw_window()
    , m_previous_selected_tab()
    , m_bounding_box()
{
    m_input_shape_controls.reserve(shapes.size());
    for (auto& shape : shapes)
        m_input_shape_controls.emplace_back(std::move(shape));
    shapes.clear();
    init_bounding_box();
    m_draw_window = std::make_unique<ShapeDrawWindow>(m_bounding_box, name);
}

ShapeWindow::~ShapeWindow() = default;

void ShapeWindow::init_bounding_box()
{
    for (const auto& shape_control : m_input_shape_controls)
        std::visit(stdutils::Overloaded {
            [this](const shapes::PointCloud2d<scalar>& s) { m_bounding_box.merge(shapes::fast_bounding_box(s)); },
            [this](const shapes::PointPath2d<scalar>& s) { m_bounding_box.merge(shapes::fast_bounding_box(s)); },
            [this](const shapes::CubicBezierPath2d<scalar>& s) { m_bounding_box.merge(shapes::fast_bounding_box(s)); },
            [](const auto&) { assert(0); }
        }, shape_control.shape);
    shapes::ensure_min_extent(m_bounding_box);
}

void ShapeWindow::recompute_triangulations()
{
    stdutils::io::ErrorHandler err_handler = [](stdutils::io::SeverityCode, std::string_view msg) { std::cerr << msg << std::endl; };
    m_triangulation_shape_controls.clear();

    std::chrono::duration<float, std::milli> duration;
    for (const auto& algo : delaunay::get_impl_list<double>())
    {
        {
            stdutils::chrono::DurationMeas meas(duration);

            // Setup triangulation
            auto triangulation_algo = algo.impl_factory();
            assert(triangulation_algo);
            bool first_path = true;
            std::vector<const ShapeControl*> active_shapes;
            for (const auto& shape_control : m_input_shape_controls)
            {
                if (shape_control.active)
                    active_shapes.emplace_back(&shape_control);
            }
            for (const auto& shape_control_ptr : m_sampled_shape_controls)
            {
                assert(shape_control_ptr);
                if (shape_control_ptr->active)
                    active_shapes.emplace_back(shape_control_ptr.get());
            }
            for (const auto* shape_control_ptr : active_shapes)
            {
                std::visit(stdutils::Overloaded {
                    [&triangulation_algo, &err_handler](const shapes::PointCloud2d<scalar>& pc) { triangulation_algo->add_steiner(pc, err_handler); },
                    [&triangulation_algo, &err_handler, &first_path](const shapes::PointPath2d<scalar>& pp) {
                        if (first_path && pp.vertices.size() >= 3) { triangulation_algo->add_path(pp, err_handler); first_path = false; }
                        else { triangulation_algo->add_hole(pp, err_handler); }
                    },
                    [](const shapes::CubicBezierPath2d<scalar>&) { /* Skip */ },
                    [](const auto&) { assert(0); }
                }, shape_control_ptr->shape);
            }
            // Triangulate
            m_triangulation_shape_controls.emplace_back(algo.name, triangulation_algo->triangulate(err_handler));
        }
        m_triangulation_shape_controls.back().second.latest_computation_time_ms = duration.count();
    }
}

ShapeDrawWindow::DrawCommandLists ShapeWindow::build_draw_lists() const
{
    ShapeDrawWindow::DrawCommandLists draw_command_lists;
    {
        auto& draw_command_list = draw_command_lists.emplace_back(INPUT_TAB_NAME, std::vector<ShapeDrawWindow::DrawCommand>()).second;
        for (const auto& shape_control : m_input_shape_controls)
        {
            if (shape_control.active)
                draw_command_list.emplace_back(to_draw_command(shape_control));
        }
        for (const auto& shape_control_ptr : m_sampled_shape_controls)
        {
            assert(shape_control_ptr);
            if (shape_control_ptr->active)
                draw_command_list.emplace_back(to_draw_command(*shape_control_ptr));
        }
    }

    for (const auto& [algo_name, shape_control_dt] : m_triangulation_shape_controls)
    {
        auto& draw_command_list = draw_command_lists.emplace_back(algo_name, std::vector<ShapeDrawWindow::DrawCommand>()).second;
        draw_command_list.emplace_back(to_draw_command(shape_control_dt));
        for (const auto& shape_control : m_input_shape_controls)
        {
            if (shape_control.active && !shapes::is_bezier_path(shape_control.shape))
            {
                auto& draw_command = draw_command_list.emplace_back(to_draw_command(shape_control));
                draw_command.constraint_edges = true;
            }
        }
        for (const auto& shape_control_ptr : m_sampled_shape_controls)
        {
            if (shape_control_ptr->active)
            {
                assert(!shapes::is_bezier_path(shape_control_ptr->shape));
                auto& draw_command = draw_command_list.emplace_back(to_draw_command(*shape_control_ptr));
                draw_command.constraint_edges = true;
            }
        }
    }
    return draw_command_lists;
}

void ShapeWindow::update_opengl_draw_list(bool geometry_has_changed, const Settings& settings)
{
    DrawingOptions options;
    options.point_settings = settings.read_point_settings();
    options.path_settings = settings.read_path_settings();
    options.surface_settings = settings.read_surface_settings();

    const std::string& current_tab = m_draw_window->get_selected_tab();
    m_renderer_draw_list.clear();

    for (const auto& [algo_name, shape_control] : m_triangulation_shape_controls)
    {
        if (algo_name == current_tab) { to_opengl_draw_commands<scalar>(shape_control, m_renderer_draw_list, options); }
    }
    options.constraint_edges = (current_tab != INPUT_TAB_NAME);
    for (const auto& shape_control : m_input_shape_controls)
    {
        if (shape_control.active) { to_opengl_draw_commands<scalar>(shape_control, m_renderer_draw_list, options); }
    }
    for (const auto& shape_control_ptr : m_sampled_shape_controls)
    {
        assert(shape_control_ptr);
        if (shape_control_ptr->active) { to_opengl_draw_commands<scalar>(*shape_control_ptr, m_renderer_draw_list, options); }
    }

    if (geometry_has_changed || current_tab != m_previous_selected_tab)
        m_renderer_draw_list.m_buffer_version++;
    m_previous_selected_tab = current_tab;
}

ShapeWindow::ShapeControl* ShapeWindow::allocate_new_sampled_shape(shapes::AllShapes<scalar>&& shape)
{
    return m_sampled_shape_controls.emplace_back(std::make_unique<ShapeControl>(std::move(shape))).get();
}

void ShapeWindow::delete_sampled_shape(ShapeControl** sc)
{
    const auto sc_it = std::find_if(std::cbegin(m_sampled_shape_controls), std::cend(m_sampled_shape_controls), [sc](const auto& elt) { return elt.get() == *sc; });
    assert(sc_it != std::cend(m_sampled_shape_controls));
    m_sampled_shape_controls.erase(sc_it);
    *sc = nullptr;
}

void ShapeWindow::shape_list_menu(ShapeControl& shape_control, unsigned int idx, bool allow_sampling, bool& geometry_has_changed)
{
    std::stringstream label;
    label << "Shape #" << idx;
    const bool is_open = ImGui::TreeNode(label.str().c_str());
    shape_control.highlight = ImGui::IsItemHovered();
    if (is_open)
    {
        // Active button
        std::stringstream active_button;
        active_button << "Active#" << idx;
        float hue = shape_control.active ? 0.3f : 0.f;
        ImGui::PushID(active_button.str().c_str());
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));
        ImGui::PushStyleColor(ImGuiCol_Text, shape_control.active ? ImVec4(1.f, 1.f, 1.f, 1.f) : ImVec4(0.4f, 0.4f, 0.4f, 1.f));
        const bool pressed = ImGui::Button("Active");
        ImGui::PopStyleColor(4);
        ImGui::PopID();
        if (pressed && !shape_control.force_inactive)
        {
            shape_control.active = !shape_control.active;
            geometry_has_changed = true;
        }

        // Info
        ImGui::Text("Nb vertices: %ld, nb edges: %ld", shape_control.nb_vertices, shape_control.nb_edges);

        // Sampling
        if (allow_sampling && (shapes::is_bezier_path(shape_control.shape) || shapes::is_point_path(shape_control.shape)))
        {
            std::stringstream sample_checkbox;
            sample_checkbox << "Sample##" << idx;
            bool is_sampled = (shape_control.sampled_shape != nullptr);
            ImGui::Checkbox(sample_checkbox.str().c_str(), &is_sampled);
            if (is_sampled && !shape_control.sampled_shape)
            {
                shape_control.sampled_shape = allocate_new_sampled_shape(shapes::trivial_sampling(shape_control.shape));
                if (std::holds_alternative<shapes::CubicBezierPath2d<scalar>>(shape_control.shape))
                {
                    const auto& cbp = std::get<shapes::CubicBezierPath2d<scalar>>(shape_control.shape);
                    shape_control.sampler = std::make_unique<shapes::UniformSamplingCubicBezier2d<scalar>>(cbp);
                    shape_control.sampling_length = static_cast<float>(shape_control.sampler->max_segment_length());
                }
                else if (std::holds_alternative<shapes::PointPath2d<scalar>>(shape_control.shape))
                {
                    const auto& pp = std::get<shapes::PointPath2d<scalar>>(shape_control.shape);
                    shape_control.active = false;
                    shape_control.force_inactive = true;
                    shape_control.sampler = std::make_unique<shapes::UniformSamplingPointPath2d<scalar>>(pp);
                    shape_control.sampling_length = static_cast<float>(shape_control.sampler->max_segment_length());
                }
                geometry_has_changed = true;
            }
            if (!is_sampled && shape_control.sampled_shape)
            {
                shape_control.sampler.reset();
                delete_sampled_shape(&shape_control.sampled_shape);
                shape_control.force_inactive = false;
                geometry_has_changed = true;
            }
            if (shape_control.sampler)
            {
                std::stringstream sampling_length_id;
                sampling_length_id << "Sampling length##" << idx;
                assert(is_sampled);
                const float max = 1.05f * static_cast<float>(shape_control.sampler->max_segment_length());
                const float min = max / 1000.f;
                float new_sampling_length = shape_control.sampling_length;
                ImGui::SliderFloat(sampling_length_id .str().c_str(), &new_sampling_length, min, max, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
                if (new_sampling_length != shape_control.sampling_length)
                {
                    shape_control.sampling_length = new_sampling_length;
                    shape_control.sampled_shape->update(shapes::AllShapes<scalar>(shape_control.sampler->sample(static_cast<scalar>(new_sampling_length))));
                    geometry_has_changed = true;
                }
            }
        }
        ImGui::TreePop();
    }
}

void ShapeWindow::visit(bool& can_be_erased, const Settings& settings)
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(600.f, 200.f), ImVec2(FLT_MAX, FLT_MAX));
    constexpr ImGuiWindowFlags win_flags = ImGuiWindowFlags_AlwaysAutoResize;
    bool is_window_open = true;
    if (!ImGui::Begin(m_title.c_str(), &is_window_open, win_flags))
    {
        // Collapsed
        can_be_erased = !is_window_open;
        ImGui::End();
        return;
    }

    // Global bounding box
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Input bounding box"))
    {
        if (ImGui::BeginTable("bounding_box_table", 3, ImGuiTableFlags_SizingStretchSame))
        {
            {
                ImGui::TableNextRow();
                const auto tl_corner = m_bounding_box.min();
                ImGui::TableNextColumn();
                ImGui::Text("Top-left corner");
                ImGui::TableNextColumn();
                ImGui::Text("%0.3g", tl_corner.x);
                ImGui::TableNextColumn();
                ImGui::Text("%0.3g", tl_corner.y);
            }
            {
                ImGui::TableNextRow();
                const auto br_corner = m_bounding_box.max();
                ImGui::TableNextColumn();
                ImGui::Text("Bottom-right corner");
                ImGui::TableNextColumn();
                ImGui::Text("%0.3g", br_corner.x);
                ImGui::TableNextColumn();
                ImGui::Text("%0.3g", br_corner.y);
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // Iterate all input/samples shapes
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Input shapes"))
    {
        unsigned int shape_idx = 1;
        for (auto& shape_control : m_input_shape_controls)
        {
            shape_list_menu(shape_control, shape_idx++, ALLOW_SAMPLING, m_geometry_has_changed);
        }
        ImGui::TreePop();
    }
    else
    {
        for (auto& shape_control : m_input_shape_controls)
        {
            shape_control.highlight = false;
        }
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Sampled shapes"))
    {
        unsigned int shape_idx = 1;
        for (auto& shape_control_ptr : m_sampled_shape_controls)
        {
            assert(shape_control_ptr);
            shape_list_menu(*shape_control_ptr, shape_idx++, !ALLOW_SAMPLING, m_geometry_has_changed);
        }
        ImGui::TreePop();
    }
    else
    {
        for (auto& shape_control_ptr : m_sampled_shape_controls)
        {
            assert(shape_control_ptr);
            shape_control_ptr->highlight = false;
        }
    }

    stdutils::io::ErrorHandler err_handler = [](stdutils::io::SeverityCode, std::string_view msg) { std::cerr << msg << std::endl; };

    // Triangulate
    if (m_geometry_has_changed)
    {
        std::chrono::duration<float, std::milli> duration;
        {
            stdutils::chrono::DurationMeas meas(duration);
            recompute_triangulations();
        }
    }

    // Triangulation shapes
    for (auto& triangulation_shape_control_pair : m_triangulation_shape_controls)
    {
        const std::string& algo_name = triangulation_shape_control_pair.first;
        auto& triangulation_shape_control = triangulation_shape_control_pair.second;
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode(algo_name.c_str()))
        {
            const bool is_open = ImGui::TreeNode("Shape #1");
            triangulation_shape_control.highlight = ImGui::IsItemHovered();
            if (is_open)
            {
                // Info
                ImGui::Text("Nb vertices: %ld, nb edges: %ld, nb faces: %ld", triangulation_shape_control.nb_vertices, triangulation_shape_control.nb_edges, triangulation_shape_control.nb_faces);
                ImGui::Text("Computation time: %0.3g ms", static_cast<double>(triangulation_shape_control.latest_computation_time_ms));
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    ImGui::End();

    // Set opengl draw list
    update_opengl_draw_list(m_geometry_has_changed, settings);
    m_geometry_has_changed = false;

    // Draw Window
    bool draw_window_can_be_erased = false;
    assert(m_draw_window);
    m_draw_window->visit(draw_window_can_be_erased, settings, build_draw_lists());

    can_be_erased = draw_window_can_be_erased || !is_window_open;
}

shapes::BoundingBox2d<ShapeWindow::scalar> ShapeWindow::get_canvas_bounding_box() const
{
    return m_draw_window ? m_draw_window->get_canvas_bounding_box() : shapes::BoundingBox2d<ShapeWindow::scalar>();
}
