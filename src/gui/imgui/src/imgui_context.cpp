// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <imgui/imgui_context.h>

#include "imgui_context_impl.h"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {

    // Global variable: Singleton representing the ImGui context and the GLFW window context
    std::unique_ptr<DearImGuiContextImpl> g_imgui_context_impl;

}

DearImGuiContextImpl* get_imgui_context_impl()
{
    return g_imgui_context_impl.get();
}

DearImGuiContext::DearImGuiContext(GLFWwindow* glfw_window, bool& any_fatal_error, FlagCode flags) noexcept
{
    any_fatal_error = false;
    try
    {
        if (glfw_window == nullptr)
        {
            any_fatal_error = true;
            return;
        }
        assert(g_imgui_context_impl.get() == nullptr);
        g_imgui_context_impl = std::make_unique<DearImGuiContextImpl>(glfw_window, any_fatal_error, flags);
    }
    catch (std::exception&)
    {
        any_fatal_error = true;
    }
}

DearImGuiContext::~DearImGuiContext()
{
    g_imgui_context_impl.reset();
}

void DearImGuiContext::backend_info(std::ostream& out) const
{
    assert(g_imgui_context_impl);
    g_imgui_context_impl->backend_info(out);
}

void DearImGuiContext::new_frame() const
{
    assert(g_imgui_context_impl);
    g_imgui_context_impl->new_frame();
}

void DearImGuiContext::render()
{
    DearImGuiContextImpl::render();
}

bool DearImGuiContext::register_main_font(stdutils::Span<const std::byte> font_buffer, std::string_view font_name, float base_font_size, bool scale_style, const stdutils::io::ErrorHandler& err_handler)
{
    assert(g_imgui_context_impl);
    return g_imgui_context_impl->register_main_font(font_buffer, font_name, base_font_size, scale_style, err_handler);
}

bool DearImGuiContext::register_main_font(std::filesystem::path fonts_folder, std::string_view font_file, float base_font_size, bool scale_style, const stdutils::io::ErrorHandler& err_handler)
{
    assert(g_imgui_context_impl);
    return g_imgui_context_impl->register_main_font(fonts_folder, font_file, base_font_size, scale_style, err_handler);
}

void DearImGuiContext::set_ui_scaling(float ui_scaling)
{
    assert(g_imgui_context_impl);
    g_imgui_context_impl->set_ui_scaling(ui_scaling);
}

void DearImGuiContext::push_font(int size_incr)
{
    assert(g_imgui_context_impl);
    g_imgui_context_impl->push_font(size_incr);
}

void DearImGuiContext::pop_font()
{
    assert(g_imgui_context_impl);
    g_imgui_context_impl->pop_font();
}

void DearImGuiContext::sleep(int ms) const
{
    assert(g_imgui_context_impl);
    g_imgui_context_impl->sleep(ms);
}

void DearImGuiContext::load_ini_settings_from_file(const fs::path& ini_file, const stdutils::io::ErrorHandler& err_handler) const noexcept
{
    assert(ImGui::GetIO().IniFilename == nullptr);
    assert(!ini_file.empty());
    if (ini_file.empty()) { return; }
    if (!fs::exists(ini_file)) { return; }
    try
    {
        using istream_iter = std::istreambuf_iterator<char>;
        const std::string ini_file_in_mem = stdutils::io::dump_txt_file_to_memory(ini_file, err_handler);
        if (ini_file_in_mem.empty()) { return; }
        ImGui::LoadIniSettingsFromMemory(ini_file_in_mem.data(), ini_file_in_mem.size());
    }
    catch(const std::exception& e)
    {
        std::stringstream out;
        out << "Error while loading Dear ImGui INI settings from file " << ini_file << ": " << e.what();            // std::quoted is used on the file path
        if (err_handler) { err_handler(stdutils::io::Severity::EXCPT, out.str()); }
    }
}

void DearImGuiContext::append_ini_settings_to_file(const fs::path& ini_file, const stdutils::io::ErrorHandler& err_handler) const noexcept
{
    assert(ImGui::GetIO().IniFilename == nullptr);
    assert(!ini_file.empty());
    if (ini_file.empty()) { return; }
    try
    {
        std::size_t ini_data_size = 0;
        const char* ini_data = ImGui::SaveIniSettingsToMemory(&ini_data_size);
        assert(ini_data);
        if (!ini_data) { return; }
        std::ios_base::openmode mode = std::ios_base::app;
        std::basic_ofstream<char> outputstream(ini_file, mode);
        if (outputstream.is_open())
        {
            outputstream << "\n# Dear ImGui\n\n";
            outputstream.write(ini_data, ini_data_size);
        }
        else
        {
            std::stringstream out;
            out << "Could not save Dear ImGui INI settings to file " << ini_file;                           // std::quoted is used on the file path
            if (err_handler) { err_handler(stdutils::io::Severity::ERR, out.str()); }
        }
    }
    catch(const std::exception& e)
    {
        std::stringstream out;
        out << "Error while saving Dear ImGui INI settings to file " << ini_file << ": " << e.what();       // std::quoted is used on the file path
        if (err_handler) { err_handler(stdutils::io::Severity::EXCPT, out.str()); }
    }
}
