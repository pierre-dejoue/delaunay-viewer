// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/io.h>
#include <stdutils/span.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>

/**
 * Manage a font at different UI scales
 *
 *  - A font file is loaded.
 *  - The default ImGui font is used as a backup if no font file was correctly loaded.
 *  - The font manager is given a scale_factor > 1.f at ctor time.
 *    That factor is raised to the power of the size_incr when pushing the font.
 *    The font size is always rounded to the closest integer
 *    For instance, with base_font_size = 13.f and a scaling_factor = 1.25f, the following applies:
 *
 *      size_incr           scaling_factor          font_size
 *    ---------------------------------------------------------
 *      push_font(-2)       0.64                    8
 *      push_font(-1)       0.80                    10
 *      push_font(0)        1.00                    13
 *      push_font(+1)       1.25                    16
 *      push_font(+2)       1.56                    20
 *
 *  - Note that the font size is then further scaled depending on the global UI scaling (this is handled in DearImGuiContext)
 *
 */
class FontManager
{
public:
    static constexpr float DEFAULT_FONT_SCALING_FACTOR = 1.25f;

    /**
     * A valid FontManager to use Dear ImGui's default font with a similar size scaling
     */
    FontManager(float scaling_factor = DEFAULT_FONT_SCALING_FACTOR);

    /**
     * Load a font from memory
     */
    FontManager(stdutils::Span<const std::byte> font_buffer, std::string_view font_name, float base_font_size, const stdutils::io::ErrorHandler& err_handler, float scaling_factor = DEFAULT_FONT_SCALING_FACTOR);

    /**
     * Attempt loading a font file. In case of failure fall-back on Dear ImGui's default font.
     */
    FontManager(std::filesystem::path fonts_folder, std::string_view font_file, float base_font_size, const stdutils::io::ErrorHandler& err_handler, float scaling_factor = DEFAULT_FONT_SCALING_FACTOR);

    ~FontManager();

    bool successfully_loaded() const noexcept;

    /**
     * Set as the new default font loaded on ImGui::NewFrame()
     */
    bool set_as_the_default_font() const noexcept;

    void push_font(int size_incr = 0);
    void pop_font();

private:
    struct Impl;
    std::unique_ptr<Impl> p_impl;
};
