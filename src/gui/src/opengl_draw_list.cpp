#include "opengl_draw_list.h"

#include "opengl_and_glfw.h"

#include <cassert>


unsigned int to_gl_draw_cmd(DrawCmd cmd)
{
    switch (cmd)
    {
        case DrawCmd::Lines:
            return GL_LINES;
        case DrawCmd::Triangles:
            return GL_TRIANGLES;
        default:
            assert(0);
            return GL_TRIANGLES;
    }
}

OpenGLDrawList::DrawCall::DrawCall()
    : m_range(0, 0)
    , m_uniform_color({1.f, 0.f, 0.f, 1.f})
    , m_cmd(DrawCmd::Lines)
{}

const OpenGLDrawList& OpenGLDrawList::empty()
{
    static OpenGLDrawList empty;
    return empty;
}

void OpenGLDrawList::clear()
{
    m_vertices.clear();
    m_indices.clear();
    m_draw_calls.clear();
}
