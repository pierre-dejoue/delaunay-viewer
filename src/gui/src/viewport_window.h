#pragma once

#include "draw_command.h"

#include <base/canvas.h>
#include <base/color_data.h>
#include <base/window_layout.h>
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
    using TabList = std::vector<Key>;
    using GeometryBB = shapes::BoundingBox2d<scalar>;
    using ViewportCanvas = Canvas<scalar>;
    using WorldCoordinatesCallback = std::function<void(const shapes::Point2d<scalar>& steiner_point)>;
    struct MouseClickTool
    {
        bool checked{false};
        WorldCoordinatesCallback callback{};
    };
    using ScrollEvent = ScreenVect;

    ViewportWindow();
    ViewportWindow(const ViewportWindow&) = delete;
    ViewportWindow& operator=(const ViewportWindow&) = delete;

    void reset();

    void visit(bool& can_be_erased, const TabList& tab_list, const Settings& settings, const WindowLayout& win_pos_sz);

    void set_geometry_bounding_box(const GeometryBB& bounding_box);

    void set_steiner_callback(const WorldCoordinatesCallback& callback);

    const Key& get_latest_selected_tab() const;

    GeometryBB get_canvas_bounding_box() const;

    ScreenBB get_viewport_bounding_box() const;

    ViewportCanvas get_viewport_canvas() const;

    const ColorData& get_background_color() const;

    void signal_scroll_event(ScrollEvent scroll_event);

private:
    struct ZoomSelectionBox
    {
        bool is_ongoing = false;
        ScreenPos corner_0;
        ScreenPos corner_1;
        bool is_positive_box() const { return corner_1.x > corner_0.x && corner_1.y > corner_0.y; }
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
    Key m_latest_selected_tab;
    ColorData m_background_color;
    MouseClickTool m_steiner_tool;
    ScrollEvent m_scroll_event;
};
