// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <imgui/font_mngr.h>

#include <imgui/imgui.h>
#include <stdutils/io.h>
#include <stdutils/type_traits.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

constexpr float FONT_SIZE_MIN{1.f};
constexpr float FONT_SIZE_MAX{100.f};

constexpr const ImFontConfig* DEFAULT_FONT_CONFIG{nullptr};

constexpr stdutils::io::SeverityCode FAIL_TO_LOAD_FONT_SEVERITY = stdutils::io::Severity::WARN;

namespace {

// ImFontAtlas::AddFontDefault() must be called once to ensure Dear ImGui's default font is loaded in the font atlas
// and can be used as the fallback font in case a font file cannot be loaded by the FontManager
ImFont* s_default_font()
{
    ImGuiIO& io = ImGui::GetIO();

    return io.Fonts->Fonts.empty()
        ? io.Fonts->AddFontDefault()
        : io.Fonts->Fonts[0];
}

ImFont* get_default_font()
{
    ImFont* const default_font = s_default_font();
    return default_font;
}

} // namespace

struct FontManager::Impl
{
private:
    void load_font(stdutils::Span<const std::byte> font_buffer, std::string_view font_name, float base_font_size, const stdutils::io::ErrorHandler& err_handler);
public:
    Impl(float scaling_factor);
    Impl(stdutils::Span<const std::byte> font_buffer, std::string_view font_name, float base_font_size, const stdutils::io::ErrorHandler& err_handler, float scaling_factor);
    Impl(fs::path fonts_folder, std::string_view font_file, float base_font_size, const stdutils::io::ErrorHandler& err_handler, float scaling_factor);
    ~Impl();

    void push_font(int size_incr);
    void pop_font();

    bool successfully_loaded() const noexcept { return success; }

    bool set_as_the_default_font() const noexcept;

    bool            success;
    ImFont*         font_ptr;
    unsigned int    pushed_fonts;
    float           base_font_size;
    float           scaling_factor;
};

void FontManager::Impl::load_font(stdutils::Span<const std::byte> font_buffer, std::string_view font_name, float base_font_size, const stdutils::io::ErrorHandler& err_handler)
{
    ImFontConfig font_config;

    // Copy the font name, limited to the size available in the ImGui font config
    constexpr std::size_t NameSZ = stdutils::array_size_v<decltype(font_config.Name)>;
    std::strncpy(font_config.Name, font_name.data(), NameSZ);
    font_config.Name[NameSZ-1] = '\0';

    ImGuiIO& io = ImGui::GetIO();
    // Transfer the ownership of the font data to Dear ImGui, which will copy the buffer. We can safely drop the const qualifier
    font_config.FontDataOwnedByAtlas = false;
    auto* font_data = const_cast<std::byte*>(font_buffer.data());
    ImFont* new_font_ptr = io.Fonts->AddFontFromMemoryTTF(reinterpret_cast<void*>(font_data), static_cast<int>(font_buffer.size()), base_font_size, &font_config, io.Fonts->GetGlyphRangesDefault());

    if (!new_font_ptr)
    {
        if (err_handler)
        {
            std::stringstream out;
            out << "Failed loading font from mem " << font_name;
            err_handler(FAIL_TO_LOAD_FONT_SEVERITY, out.str());
        }
        return;
    }

    // Finalize font loading
    success = true;
    font_ptr = new_font_ptr;
    assert(font_ptr->LegacySize == base_font_size);
    this->base_font_size = base_font_size;
    if (err_handler)
    {
        std::stringstream out;
        out << "Loaded font " << font_name;
        err_handler(stdutils::io::Severity::TRACE, out.str());
    }
}

FontManager::Impl::Impl(float scaling_factor)
    : success{false}
    , font_ptr{get_default_font()}
    , pushed_fonts{0}
    , base_font_size{0.f}
    , scaling_factor{scaling_factor}
{
    assert(font_ptr);
    base_font_size = font_ptr->LegacySize;
}

FontManager::Impl::Impl(stdutils::Span<const std::byte> font_buffer, std::string_view font_name, float base_font_size, const stdutils::io::ErrorHandler& err_handler, float scaling_factor)
    : Impl(scaling_factor)
{
    load_font(font_buffer, font_name, base_font_size, err_handler);
}

FontManager::Impl::Impl(fs::path fonts_folder, std::string_view font_file, float base_font_size, const stdutils::io::ErrorHandler& err_handler, float scaling_factor)
    : Impl(scaling_factor)
{
    ImGuiIO& io = ImGui::GetIO();
    if (!fs::is_directory(fonts_folder))
    {
        if (err_handler)
        {
            std::stringstream out;
            out << "Font directory " << fonts_folder << " not found";
            err_handler(FAIL_TO_LOAD_FONT_SEVERITY, out.str());
        }
        return;
    }
    const fs::path font_full_path = fonts_folder / font_file;
    if (!fs::exists(font_full_path))
    {
        if (err_handler)
        {
            std::stringstream out;
            out << "Font file " << font_file << " not found";
            err_handler(FAIL_TO_LOAD_FONT_SEVERITY, out.str());
        }
        return;
    }

    stdutils::FixedByteBuffer font_data = stdutils::io::dump_bin_file_to_memory(font_full_path, err_handler);

    if (font_data.empty())
    {
        if (err_handler)
        {
            std::stringstream out;
            out << "Failed loading font file " << font_file;
            err_handler(FAIL_TO_LOAD_FONT_SEVERITY, out.str());
        }
        return;
    }

    load_font(font_data.cspan(), font_file, base_font_size, err_handler);
}

FontManager::Impl::~Impl()
{
    assert(pushed_fonts == 0);
    if (success && font_ptr)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->RemoveFont(font_ptr);
    }
}

void FontManager::Impl::push_font(int size_incr)
{
    assert(font_ptr);
    const float font_size_scale = std::pow(scaling_factor, static_cast<float>(size_incr));
    const float font_size = std::clamp(std::roundf(base_font_size * font_size_scale), FONT_SIZE_MIN, FONT_SIZE_MAX);
    ImGui::PushFont(font_ptr, font_size);
    pushed_fonts++;
}

void FontManager::Impl::pop_font()
{
    if (pushed_fonts > 0)
    {
        ImGui::PopFont();
        pushed_fonts--;
    }
}

bool FontManager::Impl::set_as_the_default_font() const noexcept
{
    if (success && font_ptr)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.FontDefault = font_ptr;
    }
    return success;
}

FontManager::FontManager(float scaling_factor)
    : p_impl(std::make_unique<Impl>(scaling_factor))
{ }

FontManager::FontManager(stdutils::Span<const std::byte> font_buffer, std::string_view font_name, float base_font_size, const stdutils::io::ErrorHandler& err_handler, float scaling_factor)
    : p_impl(std::make_unique<Impl>(font_buffer, font_name, base_font_size, err_handler, scaling_factor))
{ }

FontManager::FontManager(fs::path fonts_folder, std::string_view font_file, float base_font_size, const stdutils::io::ErrorHandler& err_handler, float scaling_factor)
    : p_impl(std::make_unique<Impl>(fonts_folder, font_file, base_font_size, err_handler, scaling_factor))
{ }

FontManager::~FontManager() = default;

bool FontManager::successfully_loaded() const noexcept
{
    return p_impl->successfully_loaded();
}

bool FontManager::set_as_the_default_font() const noexcept
{
    return p_impl->set_as_the_default_font();
}

void FontManager::push_font(int size_incr)
{
    p_impl->push_font(size_incr);
}

void FontManager::pop_font()
{
    p_impl->pop_font();
}
