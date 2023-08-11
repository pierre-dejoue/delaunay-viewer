#pragma once

#include <shapes/bounding_box.h>
#include <shapes/point.h>
#include <shapes/vect.h>

#include <imgui_wrap.h>

#include <cassert>

/**
 * Canvas
 *
 * It relates two things:
 *  - A rectangular space on the screen
 *  - A world space 2D box
 */
template <typename F>
class Canvas
{
public:
    Canvas()
        : tl_corner(0.f, 0.f)
        , size(1.f, 1.f)
        , bb_corner(0.f, 0.f)
        , flip_y(true)
        , bb()
        , scale(0)
    {
        // NB: The default Canvas is invalid
    }

    Canvas(ImVec2 tl_corner, ImVec2 size, bool flip_y, shapes::BoundingBox2d<F> bb)
        : tl_corner(tl_corner)
        , size(size)
        , bb_corner()
        , flip_y(flip_y)
        , bb(bb)
        , scale(0)
    {
        assert(bb.width() > F{0});
        assert(bb.height() > F{0});
        assert(size.x > 0.f);
        assert(size.y > 0.f);

        const F scale_x = static_cast<F>(size.x) / bb.width();
        const F scale_y = static_cast<F>(size.y) / bb.height();

        if (scale_x < scale_y)
        {
            scale = scale_x;
            bb_corner = ImVec2(tl_corner.x, tl_corner.y + 0.5f * (size.y - static_cast<float>(scale * bb.height())));
        }
        else
        {
            scale = scale_y;
            bb_corner = ImVec2(tl_corner.x + 0.5f * (size.x - static_cast<float>(scale * bb.width())), tl_corner.y);
        }

        assert(scale > F{0});
    }

    F get_scale() const
    {
        return scale;
    }

    ImVec2 get_tl_corner() const
    {
        return tl_corner;
    }

    ImVec2 get_br_corner() const
    {
        return ImVec2(tl_corner.x + size.x, tl_corner.y + size.y);
    }

    ImVec2 to_screen(const shapes::Point2d<F>& p) const
    {
        assert(scale > F{0});
        //assert "is inside bb"
        return ImVec2(
            bb_corner.x + static_cast<float>(scale * (p.x - bb.rx.min)),
            bb_corner.y + static_cast<float>(scale * (flip_y ? (bb.ry.max - p.y) : (p.y - bb.ry.min)))
        );
    }

    shapes::Point2d<F> to_world(const ImVec2& p) const
    {
        assert(scale > F{0});
        return flip_y ?
            shapes::Point2d<F>(
                bb.rx.min + static_cast<F>(p.x - bb_corner.x) / scale,
                bb.ry.max - static_cast<F>(p.y - bb_corner.y) / scale) :
            shapes::Point2d<F>(
                bb.rx.min + static_cast<F>(p.x - bb_corner.x) / scale,
                bb.ry.min + static_cast<F>(p.y - bb_corner.y) / scale);
    }

    ImVec2 to_screen_vector(const shapes::Vect2d<F>& v) const
    {
        assert(scale > F{0});
        const auto sv = scale * v;
        return ImVec2(static_cast<float>(sv.x), (flip_y ? -1.f : 1.f) * static_cast<float>(sv.y));
    }

    shapes::Vect2d<F> to_world_vector(const ImVec2& v) const
    {
        assert(scale > F{0});
        return shapes::Vect2d<F>(
            static_cast<F>(v.x) / scale,
            static_cast<F>((flip_y ? -1.f : 1.f) * v.y) / scale
        );
    }

    shapes::Point2d<F> min() const
    {
        return to_world(tl_corner);
    }

    shapes::Point2d<F> max() const
    {
        return to_world(get_br_corner());
    }

private:
    // ImGui drawing canvas coordinates
    ImVec2 tl_corner;
    ImVec2 size;
    ImVec2 bb_corner;       // /!\ Not the br_corner
    bool flip_y;

    // Geometrical region
    shapes::BoundingBox2d<F> bb;

    // Conversion scale that preserve the aspect ratio
    F scale;
};

template <typename F>
bool is_valid(const Canvas<F>& canvas)
{
    return canvas.get_scale() > F{0};
}

/**
 * MouseInCanvas
 *
 * Mouse position inside a Canvas
 */
template <typename F>
struct MouseInCanvas
{
    MouseInCanvas(const Canvas<F>& canvas)
        : canvas(canvas)
        , is_hovered(false)
        , is_held(false)
        , mouse_pos()
    {}

    shapes::Point2d<F> to_world() const
    {
        assert(is_hovered);
        assert(is_valid(canvas));
        return canvas.to_world(mouse_pos);
    }

    Canvas<F> canvas;
    bool is_hovered;            // Is the mouse inside the canvas on the screen?
    bool is_held;
    ImVec2 mouse_pos;
};

