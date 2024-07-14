// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <base/canvas.h>
#include <base/color_data.h>
#include <stdutils/io.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace renderer {

enum class DrawCmd
{
    Points,
    Lines,
    Triangles,
};

class DrawList
{
public:
    using HWindex = std::uint32_t;
    using IndexRange = std::pair<std::size_t, std::size_t>;
    using VertexData = std::array<float, 3>;                    // x, y, z
    using Version = unsigned int;
    struct DrawCall
    {
        DrawCall();
        IndexRange  m_range;
        ColorData   m_uniform_color;
        float       m_uniform_point_size;
        DrawCmd     m_cmd;
    };

    Version buffer_version() const { return m_buffer_version; }

    void clear_no_reset();
    void reset();

    std::vector<VertexData>     m_vertices;
    std::vector<HWindex>        m_indices;
    std::vector<DrawCall>       m_draw_calls;

private:
    Version                     m_buffer_version{0u};           // Used to knwow when to call glBufferData
};

// Reorder commands to draw surfaces first, then lines, then points
void stable_sort_draw_commands(DrawList& draw_list);

struct Flag
{
    using type = std::uint32_t;
    static constexpr type None = 0;
    static constexpr type ViewportBackground = 1 << 0;
    static constexpr type FlipYAxis = 1 << 1;
};

/**
 * Main class of the 2D renderer
 *
 * Draw 2D primitives: Points, edges and triangles
 *
 * We distinguish two screen spaces:
 *  - The framebuffer, i.e. the whole window
 *  - The viewport, i.e. the drawing area
 */
class Draw2D
{
public:
    Draw2D(const stdutils::io::ErrorHandler* err_handler = nullptr);
    ~Draw2D();
    Draw2D(Draw2D&&) noexcept;
    Draw2D& operator=(Draw2D&&) noexcept;

    // Main init. Return true upon successful initialization.
    bool init(unsigned int back_framebuffer_id = 0);
    bool initialized() const;

    // Init and clear frambuffer(s), possibly resizing them. Return true if successful.
    bool init_framebuffer(int width, int height);
    void clear_framebuffer(ColorData clear_color);

    // Viewport background color selection
    void set_viewport_background_color(const ColorData& color);
    void set_viewport_background_color(float r, float g, float b, float a = 1.f);
    void no_viewport_background();

    // The current draw list
    DrawList& draw_list();

    // Render
    void render(const Canvas<float>& viewport_canvas, Flag::type flags = Flag::None);

    // Render only the viewport background
    void render_viewport_background(const Canvas<float>& viewport_canvas);

private:
    struct Impl;
    std::unique_ptr<Impl> p_impl;
};

} // namespace renderer
