#pragma once

#include "canvas.h"
#include "renderer.h"
#include "window_layout.h"

#include <shapes/vect.h>

#include <imgui_wrap.h>

#include <ostream>


template <typename F>
Canvas<F> build_canvas(ImVec2 tl_corner, ImVec2 size, shapes::BoundingBox2d<F> bb, bool flip_y = false)
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

void set_color(renderer::ColorData& color, ImU32 compact_color);

renderer::ColorData to_float_color(ImU32 compact_color);

namespace ImGui
{
void HelpMarker(const char* desc);          // Function taken from imgui_demo.cpp
void SetNextWindowPosAndSize(const WindowLayout& window_layout, ImGuiCond cond = 0);
}

// Do not call this class ImGuiContext because this is an internal class of Dear ImGui
struct GLFWwindow;
class DearImGuiContext
{
public:
    explicit DearImGuiContext(GLFWwindow* glfw_window, bool& any_fatal_error) noexcept;
    ~DearImGuiContext();
    DearImGuiContext(const DearImGuiContext&) = delete;
    DearImGuiContext(DearImGuiContext&&) = delete;
    DearImGuiContext& operator=(const DearImGuiContext&) = delete;
    DearImGuiContext& operator=(DearImGuiContext&&) = delete;

    void new_frame() const;
    void render() const;
    void backend_info(std::ostream& out) const;
};
