// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/io.h>
#include <stdutils/span.h>

#include <cstddef>
#include <filesystem>
#include <string_view>

/**
 * Setup the Dear ImGui context
 *
 * Do not name this class ImGuiContext because this is an internal class of Dear ImGui.
 * Instantiate this only once! Under the hood there is a singleton class, and there is only one Dear ImGui!
 */
struct GLFWwindow;
class DearImGuiContext
{
public:
    using FlagCode = unsigned int;
    struct Flag
    {
        static constexpr FlagCode None = 0;
        static constexpr FlagCode ManualINIFile = 1;            // Manage loading/saving the INI file ourselves
        static constexpr FlagCode ManualMouseCursor = 2;        // Manage the mouse cursors ourselves
    };
    explicit DearImGuiContext(GLFWwindow* glfw_window, bool& any_fatal_error, FlagCode flags = Flag::None) noexcept;
    ~DearImGuiContext();
    DearImGuiContext(const DearImGuiContext&) = delete;
    DearImGuiContext(DearImGuiContext&&) = delete;
    DearImGuiContext& operator=(const DearImGuiContext&) = delete;
    DearImGuiContext& operator=(DearImGuiContext&&) = delete;

    void backend_info(std::ostream& out) const;
    void new_frame() const;
    static void render();

    static constexpr bool SCALE_FONTS_AND_STYLE = true;
    static constexpr bool SCALE_ONLY_THE_FONTS = false;
    bool register_main_font(stdutils::Span<const std::byte> fonts_buffer, std::string_view font_name, float base_font_size, bool scale_style, const stdutils::io::ErrorHandler& err_handler);
    bool register_main_font(std::filesystem::path fonts_folder,           std::string_view font_file, float base_font_size, bool scale_style, const stdutils::io::ErrorHandler& err_handler);

    void set_ui_scaling(float ui_scaling);
    void push_font(int size_incr = 0);
    void pop_font();

    void sleep(int ms) const;

    void load_ini_settings_from_file(const std::filesystem::path& ini_file, const stdutils::io::ErrorHandler& err_handler) const noexcept;
    void append_ini_settings_to_file(const std::filesystem::path& ini_file, const stdutils::io::ErrorHandler& err_handler) const noexcept;
};
