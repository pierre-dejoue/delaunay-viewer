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

DearImGuiContext::DearImGuiContext(GLFWwindow* glfw_window, bool& any_fatal_error, FlagCode flags) noexcept
{
    any_fatal_error = false;
    try
    {
        const bool versions_ok = IMGUI_CHECKVERSION();
        const auto* ctx = ImGui::CreateContext();

        // Setup Platform/Renderer backends
        const bool init_glfw = ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
        const bool init_opengl3 = ImGui_ImplOpenGL3_Init(glsl_version());

        any_fatal_error = !versions_ok || (ctx == nullptr) || !init_glfw || !init_opengl3;

        // Manual INI save
        if (flags & Flag::ManualINIFile)
        {
            ImGui::GetIO().IniFilename = nullptr;
        }
    }
    catch(const std::exception&)
    {
        any_fatal_error = true;
    }
}

DearImGuiContext::~DearImGuiContext()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DearImGuiContext::new_frame() const
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DearImGuiContext::render() const
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DearImGuiContext::backend_info(std::ostream& out) const
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

void DearImGuiContext::sleep(int ms) const
{
    ImGui_ImplGlfw_Sleep(ms);
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
        const auto ini_file_in_mem = stdutils::io::open_and_parse_bin_file<std::vector<char>, char>(ini_file, [](std::basic_istream<char>& istream, const stdutils::io::ErrorHandler&) {
            return std::vector<char>(istream_iter(istream), istream_iter());
        }, err_handler);
        if (ini_file_in_mem.empty()) { return; }
        ImGui::LoadIniSettingsFromMemory(ini_file_in_mem.data(), ini_file_in_mem.size());
    }
    catch(const std::exception& e)
    {
        std::stringstream out;
        out << "Error while loading Dear ImGui INI settings from file [" << ini_file << "]: " << e.what();
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
            outputstream << "\n# Dear ImGui\n";
            outputstream.write(ini_data, ini_data_size);
        }
        else
        {
            std::stringstream out;
            out << "Could not save Dear ImGui INI settings to file [" << ini_file << "]";
            if (err_handler) { err_handler(stdutils::io::Severity::ERR, out.str()); }
        }
    }
    catch(const std::exception& e)
    {
        std::stringstream out;
        out << "Error while saving Dear ImGui INI settings to file [" << ini_file << "]: " << e.what();
        if (err_handler) { err_handler(stdutils::io::Severity::EXCPT, out.str()); }
    }
}
