#include "viewport_window.h"

#include "draw_shape.h"
#include "imgui_helpers.h"
#include "settings.h"

#include <shapes/bounding_box_algos.h>
#include <stdutils/macros.h>
#include <stdutils/visit.h>

#include <imgui_wrap.h>

#include <cassert>
#include <variant>


namespace
{
constexpr ViewportWindow::scalar DEFAULT_ZOOM = 0.9;
}

ViewportWindow::TabList ViewportWindow::s_default_tabs = { "<empty>" };

ViewportWindow::ViewportWindow()
    : m_title("Viewport")
    , m_initial_pos()
    , m_initial_size()
    , m_geometry_bounding_box()
    , m_canvas_bounding_box()
    , m_prev_mouse_in_canvas(Canvas<scalar>())
    , m_zoom_selection_box()
    , m_draw_command_lists()
    , m_tabs()
{
    reset();
}

void ViewportWindow::reset()
{
    // Reset tabs
    m_tabs.clear();
    m_draw_command_lists.clear();
    m_draw_command_lists.emplace(s_default_tabs.front(), DrawCommands<scalar>());

    // Reset geometry properties
    m_geometry_bounding_box.add(scalar{0}, scalar{0}).add(scalar{1}, scalar{1});
    reset_zoom();
}

void ViewportWindow::set_geometry_bounding_box(const GeometryBB& bounding_box)
{
    m_geometry_bounding_box = bounding_box;
    reset_zoom();
}

void ViewportWindow::reset_zoom()
{
    m_canvas_bounding_box = shapes::scale_around_center(m_geometry_bounding_box, DEFAULT_ZOOM);
}

void ViewportWindow::zoom_in(const shapes::BoundingBox2d<scalar>& bb)
{
    m_canvas_bounding_box = bb;  // Same as shapes::scale_around_center(bb, scalar{1});
}

void ViewportWindow::pan(const shapes::Vect2d<scalar>& dir)
{
    const auto tl_pan = m_canvas_bounding_box.min() - dir;
    const auto br_pan = m_canvas_bounding_box.max() - dir;

    m_canvas_bounding_box = shapes::BoundingBox2d<scalar>().add(tl_pan).add(br_pan);
}

void ViewportWindow::set_initial_window_pos_size(ScreenPos pos, ScreenPos size)
{
    m_initial_pos = pos;
    m_initial_size = size;
}

void ViewportWindow::set_draw_commands(Key key, const DrawCommands<scalar>& draw_commands)
{
    // The tabs are arranged in the order they are initially received
    if (m_draw_command_lists.count(key) == 0)
        m_tabs.push_back(key);

    m_draw_command_lists[key] = draw_commands;
}

void ViewportWindow::set_draw_commands(Key key, DrawCommands<scalar>&& draw_commands)
{
    // The tabs are arranged in the order they are initially received
    if (m_draw_command_lists.count(key) == 0)
        m_tabs.push_back(key);

    m_draw_command_lists[key] = std::move(draw_commands);
}

void ViewportWindow::visit(bool& can_be_erased, const Settings& settings, Key& selected_key)
{
    can_be_erased = false; // Always ON

    DrawingOptions options;
    const bool flip_y_axis = settings.read_general_settings().flip_y;
    options.point_settings = settings.read_point_settings();
    options.path_settings = settings.read_path_settings();
    options.surface_settings = settings.read_surface_settings();

    m_initial_size = to_screen_pos(ImGui::GetMainViewport()->WorkSize);
    m_initial_size.x -= m_initial_pos.x;
    m_initial_size.y -= m_initial_pos.y;
    ImGui::SetNextWindowPos(to_imgui_vec2(m_initial_pos), ImGuiCond_Once);
    ImGui::SetNextWindowSize(to_imgui_vec2(m_initial_size), ImGuiCond_Once);
    constexpr ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;

    if (!ImGui::Begin(m_title.c_str(), nullptr, win_flags))
    {
        // Collapsed
        ImGui::End();
        return;
    }

    if(ImGui::Button("Reset Zoom"))
    {
        reset_zoom();
    }

    if (is_valid(m_prev_mouse_in_canvas.canvas))
    {
        if (ImGui::BeginTable("canvas_table", 3))
        {
#if 0
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
#endif
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
    }

    if (ImGui::BeginTabBar("##TabBar"))
    {
        const auto& tabs = !m_tabs.empty() ? m_tabs : s_default_tabs;
        assert(!tabs.empty());
        for (const auto& tab_name : tabs)
            if (ImGui::BeginTabItem(tab_name.c_str(), nullptr, ImGuiTabItemFlags_None))
            {
                selected_key = tab_name;

                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                assert(draw_list);
                ImVec2 tl_corner = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                ImVec2 br_corner = ImVec2(tl_corner.x + canvas_sz.x, tl_corner.y + canvas_sz.y);

                // Canvas
                auto canvas = build_canvas(tl_corner, canvas_sz, m_canvas_bounding_box, flip_y_axis);
                MouseInCanvas mouse_in_canvas(canvas);
                ImGuiIO& io = ImGui::GetIO();
                ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
                mouse_in_canvas.canvas = canvas;
                mouse_in_canvas.is_hovered = ImGui::IsItemHovered();
                mouse_in_canvas.is_held = ImGui::IsItemActive();
                mouse_in_canvas.mouse_pos = to_screen_pos(io.MousePos);

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
                        const ScreenPos z_tl_corner(std::min(corner_0.x, corner_1.x), std::min(corner_0.y, corner_1.y));
                        const ScreenPos z_br_corner(std::max(corner_0.x, corner_1.x), std::max(corner_0.y, corner_1.y));
                        if (z_br_corner.x - z_tl_corner.x > 3.f && z_br_corner.y - z_tl_corner.y > 3.f)
                            zoom_in(shapes::BoundingBox2d<scalar>().add(canvas.to_world(z_tl_corner)).add(canvas.to_world(z_br_corner)));
                    }
                }

                // Pan
                if (mouse_in_canvas.is_held && !m_zoom_selection_box.is_ongoing && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
                {
                    pan(canvas.to_world_vector(to_screen_pos(io.MouseDelta)));
                }

                // Clip rectangle
                draw_list->PushClipRect(tl_corner, br_corner, true);

                // Render shapes with ImGui primitives
                assert(m_draw_command_lists.count(tab_name));
                for (const auto& draw_command : m_draw_command_lists[tab_name])
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
                    draw_list->AddRect(
                        to_imgui_vec2(m_zoom_selection_box.corner_0),
                        to_imgui_vec2(m_zoom_selection_box.corner_1),
                        IM_COL32(120, 120, 120, 255), rounding,
                        ImDrawFlags_None, thickness
                    );
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

ViewportWindow::GeometryBB ViewportWindow::get_canvas_bounding_box() const
{
    return m_canvas_bounding_box;
}

ViewportWindow::ScreenBB ViewportWindow::get_viewport_bounding_box() const
{
    return ScreenBB().add(m_prev_mouse_in_canvas.canvas.get_tl_corner()).add(m_prev_mouse_in_canvas.canvas.get_br_corner());
}