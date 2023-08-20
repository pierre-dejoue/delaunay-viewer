#pragma once

#include "canvas.h"
#include "renderer.h"

#include <shapes/bounding_box.h>
#include <shapes/shapes.h>
#include <shapes/vect.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>


class Settings;

// Slave window of the ShapeWindow
class ShapeDrawWindow
{
public:
    using scalar = double;
    struct DrawCommand
    {
        DrawCommand(const shapes::AllShapes<scalar>& shape);
        bool highlight;
        bool constraint_edges;
        bool geometry_updated;
        const shapes::AllShapes<scalar>* shape;
    };
    using DrawCommandList = std::pair<std::string, std::vector<DrawCommand>>;
    using DrawCommandLists = std::vector<DrawCommandList>;

public:
    ShapeDrawWindow(
        const shapes::BoundingBox2d<scalar>& bounding_box,
        std::string_view name);
    ShapeDrawWindow(const ShapeDrawWindow&) = delete;
    ShapeDrawWindow& operator=(const ShapeDrawWindow&) = delete;

    void visit(bool& can_be_erased, const Settings& settings, const DrawCommandLists& draw_command_lists);

    shapes::BoundingBox2d<scalar> get_canvas_bounding_box() const;
    const std::string& get_selected_tab() const;

private:
    struct ZoomSelectionBox
    {
        bool is_ongoing = false;
        ScreenPos corner_0;
        ScreenPos corner_1;
    };
    void reset_zoom();
    void zoom_in(const shapes::BoundingBox2d<scalar>& bb);
    void pan(const shapes::Vect2d<scalar>& dir);

    const std::string m_title;
    const shapes::BoundingBox2d<scalar> m_bounding_box;
    shapes::BoundingBox2d<scalar> m_canvas_box;
    MouseInCanvas<scalar> m_prev_mouse_in_canvas;
    ZoomSelectionBox m_zoom_selection_box;
    std::string m_selected_tab;
};
