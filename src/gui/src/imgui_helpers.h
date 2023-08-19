#pragma once

#include "canvas.h"

#include <shapes/vect.h>

#include <imgui_wrap.h>

template <typename F>
Canvas<F> build_canvas(ImVec2 tl_corner, ImVec2 size, shapes::BoundingBox2d<F> bb, bool flip_y = false)
{
    return Canvas(ScreenPos(tl_corner.x, tl_corner.y), ScreenPos(size.x, size.y), bb, flip_y);
}

inline ScreenPos to_screen_pos(ImVec2 vec2)
{
    return ScreenPos(vec2.x, vec2.y);
}

inline ImVec2 to_imgui_vec2(ScreenPos pos)
{
    return ImVec2(pos.x, pos.y);
}
