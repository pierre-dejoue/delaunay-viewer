// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <imgui/imgui.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <gui/abstract/color_data.h>
#include <gui/base/opengl_and_glfw.h>

#include <cassert>
#include <cstdlib>
#include <fstream>

void set_color(ColorData& color, ImU32 compact_color)
{
    const ImColor im_color(compact_color);
    color[0] = im_color.Value.x;
    color[1] = im_color.Value.y;
    color[2] = im_color.Value.z;
    color[3] = im_color.Value.w;
}

ColorData to_float_color(ImU32 compact_color)
{
    const ImColor im_color(compact_color);
    return ColorData{ im_color.Value.x, im_color.Value.y, im_color.Value.z, im_color.Value.w };
}

namespace ImGui {

void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// Place the window in the working area, that is the position of the viewport minus task bars, menus bars, status bars, etc.
void SetNextWindowPosAndSize(const WindowLayout& window_layout, ImGuiCond cond)
{
    const auto work_tl_corner = ImGui::GetMainViewport()->WorkPos;
    const auto work_size = ImGui::GetMainViewport()->WorkSize;
    const ImVec2 tl_corner(window_layout.pos().x + work_tl_corner.x, window_layout.pos().y + work_tl_corner.y);
    ImGui::SetNextWindowPos(tl_corner, cond);
    ImGui::SetNextWindowSize(to_imgui_vec2(window_layout.size(to_screen_size(work_size))), cond);
}

void BulletTextUnformatted(const char* txt)
{
    ImGui::BulletText("%s", txt);
}

} // namespace ImGui
