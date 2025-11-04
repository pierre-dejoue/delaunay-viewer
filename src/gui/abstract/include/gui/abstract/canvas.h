// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <shapes/bounding_box.h>
#include <shapes/point.h>
#include <shapes/vect.h>

#include <cassert>
#include <cstdint>
#include <type_traits>

// Screen space
using ScreenUnit = float;
using ScreenVect = shapes::Point2d<ScreenUnit>;
using ScreenPos = shapes::Point2d<ScreenUnit>;
using ScreenSize = shapes::Vect2d<ScreenUnit>;
using ScreenBB = shapes::BoundingBox2d<ScreenUnit>;
static_assert(std::is_floating_point_v<ScreenUnit>);

// World space
template <typename F>
using WorldPos = shapes::Point2d<F>;
template <typename F>
using WorldSpaceBB = shapes::BoundingBox2d<F>;

// Identify the corners of a frame in screen space
struct CanvasCorner
{
    using type = std::uint8_t;
    static constexpr type TOP_LEFT     = 0;
    static constexpr type TOP_RIGHT    = 1;
    static constexpr type BOTTOM_LEFT  = 2;
    static constexpr type BOTTOM_RIGHT = 3;
};
inline bool is_left_side(CanvasCorner::type c) { return (c & 1u) == 0; }
inline bool is_top_side(CanvasCorner::type c)  { return c < 2u; }
inline CanvasCorner::type opposite_corner(CanvasCorner::type c) { assert(c <= 3u); return 3u - c; }

/**
 * Canvas
 *
 * It relates two things:
 *  - A rectangular space on the screen
 *  - A world space 2D box
 *
 * This class is generic on the floating point type F used for world space coordinates and lengths.
 * Screen space's coordinates and lengths have float precision (i.e. type ScreenUnit)
 */
template <typename F>
class Canvas
{
    static_assert(std::is_floating_point_v<F>);

public:
    Canvas()
        : tl_corner(0.f, 0.f)
        , size(1.f, 1.f)
        , bb_tl_corner(0.f, 0.f)
        , flip_y(false)
        , bb()
        , scale(0)
    {
        // NB: The default Canvas is invalid
    }

    Canvas(ScreenPos tl_corner, ScreenSize size, const WorldSpaceBB<F>& bb, bool flip_y = false)
        : tl_corner(tl_corner)
        , size(size)
        , bb_tl_corner()
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
            bb_tl_corner = ScreenPos(tl_corner.x, tl_corner.y + 0.5f * (size.y - static_cast<ScreenUnit>(scale * bb.height())));
        }
        else
        {
            scale = scale_y;
            bb_tl_corner = ScreenPos(tl_corner.x + 0.5f * (size.x - static_cast<ScreenUnit>(scale * bb.width())), tl_corner.y);
        }

        assert(scale > F{0});
    }

    Canvas(ScreenUnit x, ScreenUnit y, ScreenUnit width, ScreenUnit height, const WorldSpaceBB<F>& bb, bool flip_y = false)
        : Canvas(ScreenPos(x, y), ScreenSize(width, height), bb, flip_y)
    {}

    Canvas(const ScreenBB& screen_bb, const WorldSpaceBB<F>& bb, bool flip_y = false)
        : Canvas(screen_bb.min(), screen_bb.extent(), bb, flip_y)
    {}

    Canvas(const Canvas& other, ScreenUnit screen_coordinates_scale)
        : tl_corner(screen_coordinates_scale * other.tl_corner)
        , size(screen_coordinates_scale * other.size)
        , bb_tl_corner(screen_coordinates_scale * other.bb_tl_corner)
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

    ScreenPos get_center() const
    {
        return ScreenPos(tl_corner.x + ScreenUnit{0.5} * size.x, tl_corner.y + ScreenUnit{0.5} * size.y);
    }

    ScreenBB get_screen_bounding_box() const
    {
        return ScreenBB().add(tl_corner).add(get_br_corner());
    }

    ScreenSize get_size() const
    {
        return size;
    }

    ScreenUnit to_screen(F world_length) const
    {
        return static_cast<ScreenUnit>(world_length * scale);
    }

    ScreenPos to_screen(const shapes::Point2d<F>& p) const
    {
        assert(scale > F{0});
        //assert "is inside bb"
        return ScreenPos(
            bb_tl_corner.x + static_cast<ScreenUnit>(scale * (p.x - bb.rx.min)),
            bb_tl_corner.y + static_cast<ScreenUnit>(scale * (flip_y ? (p.y - bb.ry.min) : (bb.ry.max - p.y)))
        );
    }

    F to_world(ScreenUnit screen_length) const
    {
        return static_cast<F>(screen_length) / scale;
    }

    WorldPos<F> to_world(const ScreenPos& p) const
    {
        assert(scale > F{0});
        return flip_y ?
            shapes::Point2d<F>(
                bb.rx.min + static_cast<F>(p.x - bb_tl_corner.x) / scale,
                bb.ry.min + static_cast<F>(p.y - bb_tl_corner.y) / scale) :
            shapes::Point2d<F>(
                bb.rx.min + static_cast<F>(p.x - bb_tl_corner.x) / scale,
                bb.ry.max - static_cast<F>(p.y - bb_tl_corner.y) / scale);
    }

    ScreenPos to_screen_vector(const WorldPos<F>& v) const
    {
        assert(scale > F{0});
        const auto sv = scale * v;
        return ScreenPos(static_cast<ScreenUnit>(sv.x), (flip_y ? -1.f : 1.f) * static_cast<ScreenUnit>(sv.y));
    }

    WorldPos<F> to_world_vector(const ScreenPos& v) const
    {
        assert(scale > F{0});
        return shapes::Vect2d<F>(
            static_cast<F>(v.x) / scale,
            static_cast<F>((flip_y ? 1.f : -1.f) * v.y) / scale
        );
    }

    WorldPos<F> min() const
    {
        return to_world(tl_corner);
    }

    WorldPos<F> max() const
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

    enum class Dir
    {
        Left,
        Right,
    };

    // Reframe this Canvas to produce a new Canvas that captures the same coordinates transformation
    // between the world and screen spaces, but at a different location on the screen
    Canvas reframe_in_screen_space(const ScreenBB& screen_bb) const
    {
        const auto bb_min = to_world(screen_bb.min());
        const auto bb_max = to_world(screen_bb.max());
        const auto new_bb = WorldSpaceBB<F>().add(bb_min).add(bb_max);
        return Canvas(screen_bb, new_bb, flip_y);
    }

    // TODO: Can the same be achieved using reframe_in_screen_space?
    Canvas get_half_canvas(Dir dir = Dir::Left) const
    {
        switch (dir)
        {
            case Dir::Left:
            {
                const ScreenSize half_size(std::floor(size.x / 2.f), size.y);
                const ScreenPos mid_br_corner(tl_corner.x + half_size.x, tl_corner.y + size.y);
                const shapes::Point2d<F> mid = to_world(mid_br_corner);
                const auto half_bb = WorldSpaceBB<F>().add(min()).add(mid);
                return Canvas(tl_corner, half_size, half_bb, flip_y);
            }
            case Dir::Right:
            {
                const ScreenSize half_size(std::ceil(size.x / 2.f), size.y);
                const ScreenPos mid_tl_corner(tl_corner.x + std::floor(size.x / 2.f), tl_corner.y);
                const shapes::Point2d<F> mid = to_world(mid_tl_corner);
                const auto half_bb = WorldSpaceBB<F>().add(mid).add(max());
                return Canvas(mid_tl_corner, half_size, half_bb, flip_y);
            }
            default:
                assert(0);
                return *this;
        }
    }

private:
    // Drawing canvas coordinates on the screen
    ScreenPos tl_corner;
    ScreenSize size;
    ScreenPos bb_tl_corner;     // Screen coordinates of the top-left corner of the geometry bounding box
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
    MouseInCanvas()
        : canvas()
        , is_hovered(false)
        , is_held(false)
        , mouse_pos()
    {
        // Is invalid
    }

    explicit MouseInCanvas(const Canvas<F>& canvas)
        : canvas(canvas)
        , is_hovered(false)
        , is_held(false)
        , mouse_pos()
    { }

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

template <typename F>
bool is_valid(const MouseInCanvas<F>& mouse_in_canvas)
{
    return is_valid(mouse_in_canvas.canvas);
}
