// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "imgui_context_impl.h"

#include <gui/base/opengl_and_glfw.h>
#include <imgui/imgui_context.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>

DearImGuiContextImpl::DearImGuiContextImpl(GLFWwindow* glfw_window, bool& any_fatal_error, DearImGuiContext::FlagCode flags)
    : m_glfw_window(glfw_window)
    , m_primary_font_manager()
    , m_scale_style_with_fonts{false}
    , m_ui_scaling{1.f}
    , m_initial_style()
{
    assert(glfw_window);
    const bool versions_ok = IMGUI_CHECKVERSION();
    const auto* ctx = ImGui::CreateContext();

    // A default font manager
    m_primary_font_manager = std::make_unique<FontManager>();
    assert(m_primary_font_manager);

    // Setup Platform/Renderer backends
    const bool init_glfw = ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
    const bool init_opengl3 = ImGui_ImplOpenGL3_Init(glsl_version());

    any_fatal_error = !versions_ok || (ctx == nullptr) || !init_glfw || !init_opengl3;

    // Manage the INI load/save ourselves
    if (flags & DearImGuiContext::Flag::ManualINIFile)
    {
        ImGui::GetIO().IniFilename = nullptr;
    }
}

DearImGuiContextImpl::~DearImGuiContextImpl()
{
    m_initial_style.reset();
    m_primary_font_manager.reset();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DearImGuiContextImpl::backend_info(std::ostream& out) const
{
    // Imgui
    const ImGuiIO& io = ImGui::GetIO();
    out << "Dear ImGui " << IMGUI_VERSION
        << " (Backend platform: " << (io.BackendPlatformName ? io.BackendPlatformName : "NULL")
        << ", renderer: " << (io.BackendRendererName ? io.BackendRendererName : "NULL") << ")" << '\n';

    // GLFW
    GLFWWindowContext::glfw_version_info(out);

    // OpenGL
    opengl_version_info(out);
}

void DearImGuiContextImpl::new_frame()
{
    const ImGuiMouseCursor imgui_mouse_cursor_hint = ImGui::GetMouseCursor();       // Call this before ImGui::NewFrame() because the new frame will reset the value

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DearImGuiContextImpl::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool DearImGuiContextImpl::register_main_font(stdutils::Span<const std::byte> font_buffer, std::string_view font_name, float base_font_size, bool scale_style, const stdutils::io::ErrorHandler& err_handler)
{
    m_primary_font_manager = std::make_unique<FontManager>(font_buffer, font_name, base_font_size, err_handler);
    m_scale_style_with_fonts = scale_style;
    assert(m_ui_scaling == 1.f);
    return m_primary_font_manager->set_as_the_default_font();
}

bool DearImGuiContextImpl::register_main_font(std::filesystem::path fonts_folder, std::string_view font_file, float base_font_size, bool scale_style, const stdutils::io::ErrorHandler& err_handler)
{
    m_primary_font_manager = std::make_unique<FontManager>(fonts_folder, font_file, base_font_size, err_handler);
    m_scale_style_with_fonts = scale_style;
    assert(m_ui_scaling == 1.f);
    return m_primary_font_manager->set_as_the_default_font();
}

void DearImGuiContextImpl::set_ui_scaling(float ui_scaling)
{
    // Style
    auto& imgui_style = ImGui::GetStyle();
    if (m_scale_style_with_fonts && m_ui_scaling != ui_scaling)
    {
        if (!m_initial_style)
        {
            // Freeze the original ImGui style settings
            m_initial_style = std::make_unique<ImGuiStyle>(ImGui::GetStyle());
        }
        assert(m_initial_style);
        imgui_style= *m_initial_style;
        imgui_style.ScaleAllSizes(ui_scaling);
    }
    // Fonts
    imgui_style.FontScaleDpi = ui_scaling;
    m_ui_scaling = ui_scaling;
}

void DearImGuiContextImpl::push_font(int size_incr)
{
    m_primary_font_manager->push_font(size_incr);
}

void DearImGuiContextImpl::pop_font()
{
    m_primary_font_manager->pop_font();
}

void DearImGuiContextImpl::sleep(int ms) const
{
    ImGui_ImplGlfw_Sleep(ms);
}
