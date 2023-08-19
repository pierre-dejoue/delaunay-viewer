#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>


enum class DrawCmd
{
    Lines,
    Triangles,
};

unsigned int to_gl_draw_cmd(DrawCmd cmd);

struct OpenGLDrawList
{
    using GLindex = std::uint32_t;
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

    static const OpenGLDrawList& empty();
    void clear();

    std::vector<VertexData>     m_vertices;
    std::vector<GLindex>        m_indices;
    std::vector<DrawCall>       m_draw_calls;
    Version                     m_buffer_version = 0u;          // Use to knwow when to call glBufferData
};
