#pragma once

#include "canvas.h"

#include <stdutils/io.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>


namespace renderer
{

enum class DrawCmd
{
    Lines,
    Triangles,
};

struct DrawList
{
    using HWindex = std::uint32_t;
    using IndexRange = std::pair<std::size_t, std::size_t>;
    using VertexData = std::array<float, 3>;                    // x, y, z
    using ColorData = std::array<float, 4>;                     // r, g, b, a
    using Version = unsigned int;
    struct DrawCall
    {
        DrawCall();
        IndexRange  m_range;
        ColorData   m_uniform_color;
        DrawCmd     m_cmd;
    };

    static const DrawList& empty();
    void clear();

    std::vector<VertexData>     m_vertices;
    std::vector<HWindex>        m_indices;
    std::vector<DrawCall>       m_draw_calls;
    Version                     m_buffer_version = 0u;          // Use to knwow when to call glBufferData
};

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

    // return true upon successul initialization
    bool init();

    DrawList& draw_list();

    void set_background_color(float r, float g, float b, float a = 1.f);
    void reset_background_color();

    void render(const Canvas<float>& canvas, float window_height, Flag::type flags = Flag::None);

private:
    struct Impl;
    std::unique_ptr<Impl> p_impl;
};

} // namespace renderer
