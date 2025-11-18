// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <gui/base/opengl_and_glfw.h>
#include <imgui/font_mngr.h>
#include <imgui/imgui.h>
#include <imgui/imgui_context.h>
#include <stdutils/io.h>
#include <stdutils/span.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <ostream>
#include <string_view>
#include <vector>

struct DearImGuiContextImpl
{
    DearImGuiContextImpl(GLFWwindow* glfw_window, bool& any_fatal_error, DearImGuiContext::FlagCode flags);
    ~DearImGuiContextImpl();

    void backend_info(std::ostream& out) const;
    void new_frame();
    static void render();

    bool register_main_font(stdutils::Span<const std::byte> font_buffer, std::string_view font_name, float base_font_size, bool scale_style, const stdutils::io::ErrorHandler& err_handler);
    bool register_main_font(std::filesystem::path fonts_folder,          std::string_view font_file, float base_font_size, bool scale_style, const stdutils::io::ErrorHandler& err_handler);
    void set_ui_scaling(float ui_scaling);
    void push_font(int size_incr);
    void pop_font();

    void sleep(int ms) const;


    GLFWwindow*                                 m_glfw_window;
    std::unique_ptr<FontManager>                m_primary_font_manager;
    bool                                        m_scale_style_with_fonts;
    float                                       m_ui_scaling;
    std::unique_ptr<ImGuiStyle>                 m_initial_style;
};

// Access the ImGui context singleton
DearImGuiContextImpl* get_imgui_context_impl();
