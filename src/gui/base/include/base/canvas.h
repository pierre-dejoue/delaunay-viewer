// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/bounding_box.h>
#include <shapes/point.h>
#include <shapes/vect.h>

#include <cassert>
#include <type_traits>

using ScreenPos = shapes::Vect2d<float>;
using ScreenSize = shapes::Vect2d<float>;
using ScreenBB = shapes::BoundingBox2d<float>;
template <typename F>
using WorldSpaceBB = shapes::BoundingBox2d<F>;

/**
 * Canvas
 *
 * It relates two things:
 *  - A rectangular space on the screen
 *  - A world space 2D box
 *
 * This class is generic on the floating point type F used for world space coordinates.
 * Screen space's coordinates have float precision.
 */
template <typename F>
class Canvas
{
    static_assert(std::is_floating_point_v<F>);
public:
    Canvas()
        : tl_corner(0.f, 0.f)
        , size(1.f, 1.f)
        , bb_corner(0.f, 0.f)
        , flip_y(false)
        , bb()
        , scale(0)
    {
        // NB: The default Canvas is invalid
    }

    Canvas(ScreenPos tl_corner, ScreenSize size, const WorldSpaceBB<F>& bb, bool flip_y = false)
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
            bb_corner = ScreenPos(tl_corner.x, tl_corner.y + 0.5f * (size.y - static_cast<float>(scale * bb.height())));
        }
        else
        {
            scale = scale_y;
            bb_corner = ScreenPos(tl_corner.x + 0.5f * (size.x - static_cast<float>(scale * bb.width())), tl_corner.y);
        }

        assert(scale > F{0});
    }

    Canvas(float x, float y, float width, float height, const WorldSpaceBB<F>& bb, bool flip_y = false)
        : Canvas(ScreenPos(x, y), ScreenSize(width, height), bb, flip_y)
    {}

    Canvas(const ScreenBB& screen_bb, const WorldSpaceBB<F>& bb, bool flip_y = false)
        : Canvas(screen_bb.min(), screen_bb.extent(), bb, flip_y)
    {}

    Canvas(const Canvas& other, float screen_coordinates_scale)
        : tl_corner(screen_coordinates_scale * other.tl_corner)
        , size(screen_coordinates_scale * other.size)
        , bb_corner(screen_coordinates_scale * other.bb_corner)
        , flip_y(other.flip_y)
        , bb(other.bb)
        , scale(static_cast<F>(screen_coordinates_scale) * other.scale)
    {
        assert(screen_coordinates_scale > 0);
    }

    F get_scale() const
    {
        return scale;
    }

    bool get_flip_y() const
    {
        return flip_y;
    }

    ScreenPos get_tl_corner() const
    {
        return tl_corner;
    }

    ScreenPos get_br_corner() const
    {
        return ScreenPos(tl_corner.x + size.x, tl_corner.y + size.y);
    }

    ScreenBB get_screen_bounding_box() const
    {
        return ScreenBB().add(tl_corner).add(get_br_corner());
    }

    ScreenSize get_size() const
    {
        return size;
    }

    F to_screen(const F& length) const
    {
        return length * scale;
    }

    ScreenPos to_screen(const shapes::Point2d<F>& p) const
    {
        assert(scale > F{0});
        //assert "is inside bb"
        return ScreenPos(
            bb_corner.x + static_cast<float>(scale * (p.x - bb.rx.min)),
            bb_corner.y + static_cast<float>(scale * (flip_y ? (p.y - bb.ry.min) : (bb.ry.max - p.y)))
        );
    }

    F to_world(const F& length) const
    {
        return length / scale;
    }

    shapes::Point2d<F> to_world(const ScreenPos& p) const
    {
        assert(scale > F{0});
        return flip_y ?
            shapes::Point2d<F>(
                bb.rx.min + static_cast<F>(p.x - bb_corner.x) / scale,
                bb.ry.min + static_cast<F>(p.y - bb_corner.y) / scale) :
            shapes::Point2d<F>(
                bb.rx.min + static_cast<F>(p.x - bb_corner.x) / scale,
                bb.ry.max - static_cast<F>(p.y - bb_corner.y) / scale);
    }

    ScreenPos to_screen_vector(const shapes::Vect2d<F>& v) const
    {
        assert(scale > F{0});
        const auto sv = scale * v;
        return ScreenPos(static_cast<float>(sv.x), (flip_y ? -1.f : 1.f) * static_cast<float>(sv.y));
    }

    shapes::Vect2d<F> to_world_vector(const ScreenPos& v) const
    {
        assert(scale > F{0});
        return shapes::Vect2d<F>(
            static_cast<F>(v.x) / scale,
            static_cast<F>((flip_y ? 1.f : -1.f) * v.y) / scale
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

    // The bounding box of the geometry
    const WorldSpaceBB<F>& geometry_bounding_box() const
    {
        return bb;
    }

    // The bounding box exactly matching the drawing canvas
    WorldSpaceBB<F> actual_bounding_box() const
    {
        return WorldSpaceBB<F>().add(min()).add(max());
    }

    bool operator==(const Canvas<F>& other) const
    {
        return tl_corner == other.tl_corner
            && size == other.size
            && flip_y == other.flip_y
            && bb == other.bb;
    }

private:
    // Drawing canvas coordinates on the screen
    ScreenPos tl_corner;
    ScreenSize size;
    ScreenPos bb_corner;        // /!\ Not the br_corner
    bool flip_y;                // If false, the Y-axis of the world space is in the "up" direction.

    // Geometrical region (world space)
    WorldSpaceBB<F> bb;

    // Conversion scale that preserves the aspect ratio
    F scale;
};

template <typename F>
bool is_valid(const Canvas<F>& canvas)
{
    return canvas.get_scale() > F{0};
}

template <typename F0, typename F1>
Canvas<F1> cast(const Canvas<F0>& canvas_src)
{
    const auto bb_tgt = shapes::cast<F0, F1>(canvas_src.geometry_bounding_box());
    return Canvas<F1>(
        canvas_src.get_tl_corner(),
        canvas_src.get_size(),
        bb_tgt,
        canvas_src.get_flip_y()
    );
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
    ScreenPos mouse_pos;
};

