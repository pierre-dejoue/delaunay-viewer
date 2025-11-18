// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

/*********************************************************
 *
 * Dear ImGui + helper functions
 *
 *********************************************************/
#if defined(__GNUC__)
#pragma GCC system_header
#endif

#include <imgui.h>

#include <gui/abstract/canvas.h>
#include <gui/abstract/color_data.h>
#include <gui/abstract/window_layout.h>
#include <stdutils/io.h>

#include <filesystem>
#include <ostream>

namespace fs = std::filesystem;

template <typename F>
Canvas<F> build_canvas(ImVec2 tl_corner, ImVec2 size, WorldSpaceBB<F> bb, bool flip_y = false)
{
    return Canvas<F>(ScreenPos(tl_corner.x, tl_corner.y), ScreenPos(size.x, size.y), bb, flip_y);
}

inline ScreenPos to_screen_pos(ImVec2 vec2)
{
    return ScreenPos(vec2.x, vec2.y);
}

inline ScreenSize to_screen_size(ImVec2 vec2)
{
    return ScreenSize(vec2.x, vec2.y);
}

inline ImVec2 to_imgui_vec2(ScreenPos pos)
{
    return ImVec2(pos.x, pos.y);
}

void set_color(ColorData& color, ImU32 compact_color);

ColorData to_float_color(ImU32 compact_color);

namespace ImGui {

constexpr char* NO_SHORTCUT = nullptr;
constexpr bool* NO_POPEN = nullptr;
constexpr bool NOT_SELECTED = false;

inline ImVec2 GetMouseDelta() { return ImGui::GetIO().MouseDelta; }
void HelpMarker(const char* desc);          // Function taken from imgui_demo.cpp
void SetNextWindowPosAndSize(const WindowLayout& window_layout, ImGuiCond cond = 0);
void BulletTextUnformatted(const char* txt);

} // namespace ImGui
