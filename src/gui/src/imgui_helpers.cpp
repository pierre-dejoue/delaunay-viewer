#include "imgui_helpers.h"

void set_color(renderer::ColorData& color, ImU32 compact_color)
{
    const ImColor im_color(compact_color);
    color[0] = im_color.Value.x;
    color[1] = im_color.Value.y;
    color[2] = im_color.Value.z;
    color[3] = im_color.Value.w;
}

renderer::ColorData to_float_color(ImU32 compact_color)
{
    const ImColor im_color(compact_color);
    return renderer::ColorData{ im_color.Value.x, im_color.Value.y, im_color.Value.z, im_color.Value.w };
}
