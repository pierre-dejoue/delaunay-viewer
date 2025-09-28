// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "gui_style.h"

#include <imgui/imgui.h>

namespace {
    constexpr ImColor WindowBackgroundColor_Classic(29, 75, 99, 217);
    constexpr ImColor WindowBackgroundColor_Dark(4, 8, 25, 240);
    constexpr ImColor WindowMainBackgroundColor_Classic(35, 92, 121, 255);
    constexpr ImColor WindowMainBackgroundColor_Dark(10, 25, 50, 255);
}

namespace gui_style {

ColorData get_window_background_color(bool dark_mode)
{
    const auto c = static_cast<ImVec4>(dark_mode ? WindowMainBackgroundColor_Dark : WindowMainBackgroundColor_Classic);
    return ColorData{ c.x, c.y, c.z, c.w };
}

} // namespace gui_style

void imgui_set_style(bool dark_mode)
{
    if (dark_mode)
    {
        ImGui::StyleColorsDark();
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = WindowBackgroundColor_Dark;
    }
    else
    {
        ImGui::StyleColorsClassic();
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = WindowBackgroundColor_Classic;
    }
}
