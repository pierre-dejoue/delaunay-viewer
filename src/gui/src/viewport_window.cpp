#include "viewport_window.h"

#include "drawing_settings.h"
#include "imgui_helpers.h"
#include "settings.h"

#include <shapes/bounding_box_algos.h>
#include <stdutils/macros.h>
#include <stdutils/visit.h>

#include <imgui_wrap.h>

#include <cassert>
#include <variant>

namespace {
constexpr ViewportWindow::scalar DEFAULT_ZOOM = 0.9;

constexpr ImU32 CanvasBackgroundColor_Default = IM_COL32(40, 40, 40, 255);
constexpr ImU32 CanvasBorderColor_Default =     IM_COL32(250, 250, 250, 255);

void draw_canvas_foreground(ImDrawList* draw_list, ImVec2 tl_corner, ImVec2 br_corner)
{
    assert(draw_list);
    draw_list->AddRect(tl_corner, br_corner, CanvasBorderColor_Default);
}

}

ViewportWindow::TabList ViewportWindow::s_default_tabs = { "<empty>" };

ViewportWindow::ViewportWindow()
    : m_title("Viewport")
    , m_geometry_bounding_box()
    , m_canvas_bounding_box()
    , m_prev_mouse_in_canvas(Canvas<scalar>())
    , m_zoom_selection_box()
    , m_draw_command_lists()
    , m_tabs()
    , m_latest_selected_tab()
    , m_background_color(to_float_color(CanvasBackgroundColor_Default))
    , m_steiner_checked(false)
    , m_steiner_callback()
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

    // Misc
    m_steiner_checked = false;
}

void ViewportWindow::set_geometry_bounding_box(const GeometryBB& bounding_box)
{
    m_geometry_bounding_box = bounding_box;
    reset_zoom();
}

void ViewportWindow::set_steiner_callback(const ViewportWindow::SteinerCallback& callback)
{
    m_steiner_callback = callback;
    if (!static_cast<bool>(m_steiner_callback)) { m_steiner_checked = false; }
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

void ViewportWindow::visit(bool& can_be_erased, const Settings& settings, const WindowLayout& win_pos_sz)
{
    can_be_erased = false; // Always ON

    const bool flip_y_axis = settings.read_general_settings().flip_y;
    const DrawingOptions options = drawing_options_from_settings(settings);

    ImGui::SetNextWindowPosAndSize(win_pos_sz);
    ImGui::SetNextWindowBgAlpha(0.f);           // Not using ImGuiWindowFlags_NoBackground here because this also removes the window outer border
    constexpr ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoBringToFrontOnFocus
                                         | ImGuiWindowFlags_NoCollapse
                                         | ImGuiWindowFlags_NoMove
                                         | ImGuiWindowFlags_NoResize
                                         | ImGuiWindowFlags_NoSavedSettings;

    if (!ImGui::Begin(m_title.c_str(), nullptr, win_flags))
    {
        // Collapsed
        ImGui::End();
        return;
    }

    // Pick background color
    ImGui::ColorEdit3("Background color", m_background_color.data(), ImGuiColorEditFlags_NoInputs);

    ImGui::SameLine(0, 30);
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

    if(ImGui::Button("Reset Zoom"))
    {
        reset_zoom();
    }

    if (m_steiner_callback)
    {
        ImGui::SameLine(0, 30);
        ImGui::Checkbox("Add Steiner", &m_steiner_checked);
        ImGui::SameLine(0);
        ImGui::HelpMarker("Right click to add Steiner points");
    }
    assert(static_cast<bool>(m_steiner_callback) || !m_steiner_checked);

    if (ImGui::BeginTabBar("##TabBar"))
    {
        const auto& tabs = !m_tabs.empty() ? m_tabs : s_default_tabs;
        assert(!tabs.empty());
        for (const auto& tab_name : tabs)
            if (ImGui::BeginTabItem(tab_name.c_str()))
            {
                m_latest_selected_tab = tab_name;

                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                assert(draw_list);
                ImVec2 tl_corner = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                if (canvas_sz.x <= 0.f || canvas_sz.y <= 0.f)
                {
                    // Most likely scenario is the window is iconified.
                    ImGui::EndTabItem();
                    continue;
                }
                ImVec2 br_corner = ImVec2(tl_corner.x + canvas_sz.x, tl_corner.y + canvas_sz.y);

                // Canvas
                const auto canvas = build_canvas(tl_corner, canvas_sz, m_canvas_bounding_box, flip_y_axis);
                MouseInCanvas mouse_in_canvas(canvas);
                ImGuiIO& io = ImGui::GetIO();
                ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
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
                if (!m_steiner_checked && mouse_in_canvas.is_held && !m_zoom_selection_box.is_ongoing && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
                {
                    pan(canvas.to_world_vector(to_screen_pos(io.MouseDelta)));
                }

                // Add Steiner point
                if (m_steiner_checked && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    m_steiner_callback(canvas.to_world(to_screen_pos(io.MousePos)));
                }

                // Clip rectangle
                draw_list->PushClipRect(tl_corner, br_corner, true);

                // Render shapes with ImGui primitives
                assert(m_draw_command_lists.count(tab_name));
                update_imgui_draw_list<scalar>(*draw_list, m_draw_command_lists[tab_name], canvas, options);

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

const ViewportWindow::Key& ViewportWindow::get_latest_selected_tab() const
{
    return m_latest_selected_tab;
}

ViewportWindow::GeometryBB ViewportWindow::get_canvas_bounding_box() const
{
    return m_prev_mouse_in_canvas.canvas.geometry_bounding_box();
}

ViewportWindow::ScreenBB ViewportWindow::get_viewport_bounding_box() const
{
    return ScreenBB().add(m_prev_mouse_in_canvas.canvas.get_tl_corner()).add(m_prev_mouse_in_canvas.canvas.get_br_corner());
}

const renderer::ColorData& ViewportWindow::get_background_color() const
{
    return m_background_color;
}
