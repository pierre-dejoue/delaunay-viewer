#include "draw_shape.h"

#include <algorithm>
#include <cassert>
#include <vector>


namespace
{
    constexpr ImU32 CanvasBackgroundColor_Default   = IM_COL32(40, 40, 40, 255);
    constexpr ImU32 CanvasBorderColor_Default       = IM_COL32(250, 250, 250, 255);

    constexpr ImU32 VertexColor_Default     = IM_COL32(20, 90, 116, 255);
    constexpr ImU32 VertexColor_Highlighted = IM_COL32(190, 230, 255, 255);

    constexpr ImU32 EdgeColor_Default       = IM_COL32(91, 94, 137, 255);
    constexpr ImU32 EdgeColor_Constraint    = IM_COL32(222, 91, 94, 255);
    constexpr ImU32 EdgeColor_Highlighted   = IM_COL32(230, 230, 255, 255);

    constexpr ImU32 FaceColor_Default       = IM_COL32(80, 82, 105, 255);
    constexpr ImU32 FaceColor_Highlighted   = IM_COL32(190, 230, 255, 255);
}

void draw_canvas_background(ImDrawList* draw_list, ImVec2 tl_corner, ImVec2 br_corner)
{
    assert(draw_list);
    draw_list->AddRectFilled(tl_corner, br_corner, CanvasBackgroundColor_Default);
}

void draw_canvas_foreground(ImDrawList* draw_list, ImVec2 tl_corner, ImVec2 br_corner)
{
    assert(draw_list);
    draw_list->AddRect(tl_corner, br_corner, CanvasBorderColor_Default);
}

ImU32 get_vertex_color(const DrawingOptions& options)
{
    return (options.highlight ? VertexColor_Highlighted : VertexColor_Default);
}

ImU32 get_edge_color(const DrawingOptions& options)
{
    if (options.highlight)
        return EdgeColor_Highlighted;
    else if (options.constraint_edges)
        return EdgeColor_Constraint;
    else
        return EdgeColor_Default;
}

ImU32 get_face_color(const DrawingOptions& options)
{
    const float alpha = std::clamp(options.surface_settings.alpha, 0.f, 1.f);
    const ImU32 base_color = (options.highlight ? FaceColor_Highlighted : FaceColor_Default);
    const ImU32 alpha_255 = static_cast<ImU32>(255.f * alpha);
    assert(alpha_255 <= 255);
    return ((base_color & ~IM_COL32_A_MASK) | (alpha_255 << IM_COL32_A_SHIFT));
}

void set_opengl_color(renderer::DrawList::ColorData& gl_color, ImU32 color)
{
    const ImColor im_color(color);
    gl_color[0] = im_color.Value.x;
    gl_color[1] = im_color.Value.y;
    gl_color[2] = im_color.Value.z;
    gl_color[3] = im_color.Value.w;
}
