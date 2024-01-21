#pragma once

#include "canvas.h"
#include "draw_command.h"
#include "renderer.h"
#include "window_layout.h"

#include <shapes/bounding_box.h>
#include <shapes/point.h>
#include <shapes/shapes.h>
#include <shapes/vect.h>

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


class Settings;

// Slave window of the ShapeWindow
class ViewportWindow
{
public:
    using scalar = double;
    using Key = std::string;
    using GeometryBB = shapes::BoundingBox2d<scalar>;
    using ScreenBB = shapes::BoundingBox2d<float>;
    using SteinerCallback = std::function<void(const shapes::Point2d<scalar>& steiner_point)>;

    ViewportWindow();
    ViewportWindow(const ViewportWindow&) = delete;
    ViewportWindow& operator=(const ViewportWindow&) = delete;

    void reset();

    void visit(bool& can_be_erased, const Settings& settings, const WindowLayout& win_pos_sz);

    void set_draw_commands(Key key, const DrawCommands<scalar>& draw_commands);
    void set_draw_commands(Key key, DrawCommands<scalar>&& draw_commands);

    void set_geometry_bounding_box(const GeometryBB& bounding_box);

    void set_steiner_callback(const SteinerCallback& callback);

    const Key& get_latest_selected_tab() const;

    GeometryBB get_canvas_bounding_box() const;

    ScreenBB get_viewport_bounding_box() const;

    const renderer::ColorData& get_background_color() const;

private:
    using DrawCommandLists = std::map<Key, DrawCommands<scalar>>;
    using TabList = std::vector<Key>;

    struct ZoomSelectionBox
    {
        bool is_ongoing = false;
        ScreenPos corner_0;
        ScreenPos corner_1;
    };
    void reset_zoom();
    void zoom_in(const shapes::BoundingBox2d<scalar>& bb);
    void pan(const shapes::Vect2d<scalar>& dir);

    static TabList s_default_tabs;

    const std::string m_title;
    GeometryBB m_geometry_bounding_box;
    GeometryBB m_canvas_bounding_box;
    MouseInCanvas<scalar> m_prev_mouse_in_canvas;
    ZoomSelectionBox m_zoom_selection_box;
    DrawCommandLists m_draw_command_lists;
    TabList m_tabs;
    Key m_latest_selected_tab;
    renderer::ColorData m_background_color;
    bool m_steiner_checked;
    SteinerCallback m_steiner_callback;
};
