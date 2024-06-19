// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include "canvas.h"
#include "color_data.h"

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

struct DrawList
{
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

    static const DrawList& empty();
    void clear();
    void reset();

    std::vector<VertexData>     m_vertices;
    std::vector<HWindex>        m_indices;
    std::vector<DrawCall>       m_draw_calls;
    Version                     m_buffer_version = 0u;          // Used to knwow when to call glBufferData
};

// Reorder commands to draw surfaces first, then lines, then points
void stable_sort_draw_commands(DrawList& draw_list);

struct Flag
{
    using type = std::uint32_t;
    static constexpr type None = 0;
    static constexpr type OnlyBackground = 1 << 0;
    static constexpr type FlipYAxis = 1 << 1;
};

class Draw2D
{
public:
    Draw2D(const stdutils::io::ErrorHandler* err_handler = nullptr);
    ~Draw2D();

    // Main init and framebuffer init. Return true upon successful initialization
    bool init(unsigned int back_framebuffer_id = 0);
    bool init_framebuffer(int width, int height);

    DrawList& draw_list();

    void set_background_color(const ColorData& color);
    void set_background_color(float r, float g, float b, float a = 1.f);
    void reset_background_color();

    void render(const Canvas<float>& canvas, Flag::type flags = Flag::None);

private:
    struct Impl;
    std::unique_ptr<Impl> p_impl;
};

} // namespace renderer
