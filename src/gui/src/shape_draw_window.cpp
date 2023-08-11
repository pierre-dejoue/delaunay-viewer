#include "shape_draw_window.h"

#include "draw_shape.h"
#include "settings.h"

#include <shapes/bounding_box_algos.h>
#include <stdutils/macros.h>
#include <stdutils/visit.h>

#include <imgui_wrap.h>

#include <cassert>
#include <variant>


namespace
{
constexpr ShapeDrawWindow::scalar DEFAULT_ZOOM = 0.9;
}

ShapeDrawWindow::DrawCommand::DrawCommand(const shapes::AllShapes<scalar>& shape)
    : highlight(false)
    , constraint_edges(false)
    , shape(&shape)
{
}

ShapeDrawWindow::ShapeDrawWindow(
        const shapes::BoundingBox2d<scalar>& bounding_box,
        std::string_view name)
    : m_title(std::string(name) + " Display")
    , m_bounding_box(bounding_box)
    , m_canvas_box()
    , m_prev_mouse_in_canvas(Canvas<scalar>())
    , m_zoom_selection_box()
{
    reset_zoom();
}

void ShapeDrawWindow::reset_zoom()
{
    m_canvas_box = shapes::scale_around_center(m_bounding_box, DEFAULT_ZOOM);
}

void ShapeDrawWindow::zoom_in(const shapes::BoundingBox2d<scalar>& bb)
{
    m_canvas_box = bb;  // Same as shapes::scale_around_center(bb, scalar{1});
}

void ShapeDrawWindow::pan(const shapes::Vect2d<scalar>& dir)
{
    const auto tl_pan = m_canvas_box.min() - dir;
    const auto br_pan = m_canvas_box.max() - dir;

    m_canvas_box = shapes::BoundingBox2d<scalar>().add(tl_pan).add(br_pan);
}


void ShapeDrawWindow::visit(bool& can_be_erased, const Settings& settings, const DrawCommandLists& draw_command_lists)
{
    DrawingOptions options;
    const bool flip_y_axis = settings.read_general_settings().flip_y;
    options.point_settings = settings.read_point_settings();
    options.path_settings = settings.read_path_settings();
    options.surface_settings = settings.read_surface_settings();

    ImGui::SetNextWindowSizeConstraints(ImVec2(200.f, 200.f), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowSize(ImVec2(600.f, 600.f), ImGuiCond_Once);

    constexpr ImGuiWindowFlags win_flags = ImGuiWindowFlags_None;
    bool is_window_open = true;
    if (!ImGui::Begin(m_title.c_str(), &is_window_open, win_flags))
    {
        // Collapsed
        can_be_erased = !is_window_open;
        ImGui::End();
        return;
    }
    can_be_erased = !is_window_open;

    if(ImGui::Button("Reset Zoom"))
    {
        reset_zoom();
    }

    if (is_valid(m_prev_mouse_in_canvas.canvas))
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Canvas"))
        {
            if (ImGui::BeginTable("canvas_table", 3))
            {
                {
                    ImGui::TableNextRow();
                    const auto tl_corner_world = m_prev_mouse_in_canvas.canvas.min();
                    ImGui::TableNextColumn();
                    ImGui::Text("Top-left corner");
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3g", tl_corner_world.x);
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3g", tl_corner_world.y);
                }
                {
                    ImGui::TableNextRow();
                    const auto br_corner_world = m_prev_mouse_in_canvas.canvas.max();
                    ImGui::TableNextColumn();
                    ImGui::Text("Bottom-right corner");
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3g", br_corner_world.x);
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3g", br_corner_world.y);
                }
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Mouse pointer");
                    if (m_prev_mouse_in_canvas.is_hovered)
                    {
                        assert(is_valid(m_prev_mouse_in_canvas.canvas));
                        const auto p = m_prev_mouse_in_canvas.to_world();
                        ImGui::TableNextColumn();
                        ImGui::Text("%0.3g", p.x);
                        ImGui::TableNextColumn();
                        ImGui::Text("%0.3g", p.y);
                    }
                }
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }

    if (ImGui::BeginTabBar("##TabBar"))
    {
        for (const auto& draw_commands_pair : draw_command_lists)
            if (ImGui::BeginTabItem(draw_commands_pair.first.c_str(), nullptr, ImGuiTabItemFlags_None))
            {
                const auto& draw_commands = draw_commands_pair.second;

                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                assert(draw_list);
                ImVec2 tl_corner = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                ImVec2 br_corner = ImVec2(tl_corner.x + canvas_sz.x, tl_corner.y + canvas_sz.y);

                // Canvas
                Canvas canvas(tl_corner, canvas_sz, flip_y_axis, m_canvas_box);
                MouseInCanvas mouse_in_canvas(canvas);
                ImGuiIO& io = ImGui::GetIO();
                ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
                mouse_in_canvas.canvas = canvas;
                mouse_in_canvas.is_hovered = ImGui::IsItemHovered();
                mouse_in_canvas.is_held = ImGui::IsItemActive();
                mouse_in_canvas.mouse_pos = io.MousePos;

                // Zoom selection box
                if (mouse_in_canvas.is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_zoom_selection_box.is_ongoing)
                {
                    m_zoom_selection_box.is_ongoing = true;
                    m_zoom_selection_box.corner_0 = m_zoom_selection_box.corner_1 = mouse_in_canvas.mouse_pos;
                }
                if (m_zoom_selection_box.is_ongoing)
                {
                    m_zoom_selection_box.corner_1 = mouse_in_canvas.mouse_pos;
                    if (!mouse_in_canvas.is_hovered || !ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        m_zoom_selection_box.is_ongoing = false;
                        m_zoom_selection_box.corner_1 = mouse_in_canvas.mouse_pos;
                        const auto& corner_0 = m_zoom_selection_box.corner_0;
                        const auto& corner_1 = m_zoom_selection_box.corner_1;
                        const ImVec2 z_tl_corner = ImVec2(std::min(corner_0.x, corner_1.x), std::min(corner_0.y, corner_1.y));
                        const ImVec2 z_br_corner = ImVec2(std::max(corner_0.x, corner_1.x), std::max(corner_0.y, corner_1.y));
                        if (z_br_corner.x - z_tl_corner.x > 3.f && z_br_corner.y - z_tl_corner.y > 3.f)
                            zoom_in(shapes::BoundingBox2d<scalar>().add(canvas.to_world(z_tl_corner)).add(canvas.to_world(z_br_corner)));
                    }
                }

                // Pan
                if (mouse_in_canvas.is_held && !m_zoom_selection_box.is_ongoing && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
                {
                    pan(canvas.to_world_vector(io.MouseDelta));
                }

                // Clip rectangle and background
                draw_list->PushClipRect(tl_corner, br_corner, true);
                draw_canvas_background(draw_list, tl_corner, br_corner);

                // Draw shapes
                for (const auto& draw_command : draw_commands)
                {
                    assert(draw_command.shape);
                    options.highlight = draw_command.highlight;
                    options.constraint_edges = draw_command.constraint_edges;
                    std::visit(stdutils::Overloaded {
                        [&draw_list, &canvas, &options](const shapes::PointCloud2d<scalar>& pc) { draw_point_cloud(pc, draw_list, canvas, options); },
                        [&draw_list, &canvas, &options](const shapes::PointPath2d<scalar>& pp) { draw_point_path(pp, draw_list, canvas, options); },
                        [&draw_list, &canvas, &options](const shapes::CubicBezierPath2d<scalar>& cbp) { draw_cubic_bezier_path(cbp, draw_list, canvas, options); },
                        [&draw_list, &canvas, &options](const shapes::Triangles2d<scalar>& tri) { draw_triangles(tri, draw_list, canvas, options); },
                        [](const auto&) { assert(0); }
                    }, *draw_command.shape);
                }

                // TODO : highlight bounding box

                // Zoom rectangle
                if (m_zoom_selection_box.is_ongoing)
                {
                    constexpr float rounding = 0.f;
                    constexpr float thickness = 0.5f;
                    draw_list->AddRect(m_zoom_selection_box.corner_0, m_zoom_selection_box.corner_1, IM_COL32(120, 120, 120, 255), rounding, ImDrawFlags_None, thickness);
                };

                // Canvas frame
                draw_canvas_foreground(draw_list, tl_corner, br_corner);
                draw_list->PopClipRect();

                // Store for next frame
                m_prev_mouse_in_canvas = mouse_in_canvas;

                ImGui::EndTabItem();
            }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
