#include "renderer.h"

#include "opengl_and_glfw.h"

#include <lin/mat.h>

#include <array>
#include <cassert>


namespace renderer
{

namespace
{

const char* vertex_shader_source = R"SRC(

layout (location = 0) in vec3 v_pos;
uniform mat4 mat_proj;
uniform vec4 uni_color;
uniform float pt_size;
out vec4 color;
void main()
{
    gl_Position = mat_proj * vec4(v_pos, 1.0);
    gl_PointSize = pt_size;
    color = uni_color;
}

)SRC";

const char* fragment_shader_source = R"SRC(

in vec4 color;
layout (location = 0) out vec4 out_color;
void main()
{
    out_color = color;
}

)SRC";

unsigned int to_gl_draw_cmd(DrawCmd cmd)
{
    switch (cmd)
    {
        case renderer::DrawCmd::Points:
            return GL_POINTS;
        case renderer::DrawCmd::Lines:
            return GL_LINES;
        case renderer::DrawCmd::Triangles:
            return GL_TRIANGLES;
        default:
            assert(0);
            return GL_TRIANGLES;
    }
}

}  // Anonymous namespace

DrawList::DrawCall::DrawCall()
    : m_range(0, 0)
    , m_uniform_color({1.f, 0.f, 0.f, 1.f})
    , m_uniform_point_size(1.f)
    , m_cmd(renderer::DrawCmd::Lines)
{}

const DrawList& DrawList::empty()
{
    static DrawList empty;
    return empty;
}

void DrawList::clear()
{
    m_vertices.clear();
    m_indices.clear();
    m_draw_calls.clear();
}

void DrawList::reset()
{
    clear();
    m_buffer_version++;
}

// OpenGL Implementation
struct Draw2D::Impl
{
    static inline constexpr unsigned int N_VAOS = 2u;
    static inline constexpr unsigned int N_BUFFERS = 3u;

    struct GLlocations
    {
        GLuint mat_proj = 0u;
        GLuint uni_color = 0u;
        GLuint pt_size = 0u;
        GLuint v_pos = 0u;
    };

    Impl(const stdutils::io::ErrorHandler* err_handler)
        : draw_list()
        , draw_list_last_buffer_version(0u)
        , gl_program_id(0u)
        , gl_vaos()
        , gl_buffers()
        , gl_locations()
        , has_background(false)
        , background_vertices()
        , background_color{ 0.f, 0.f, 0.f, 1.f }
        , err_handler(err_handler)
    {
    }

    ~Impl()
    {
        if (gl_program_id != 0u) { glDeleteProgram(gl_program_id); }
        glDeleteBuffers(N_BUFFERS, &gl_buffers[0]);
        glDeleteVertexArrays(N_VAOS, &gl_vaos[0]);
    }

    bool init()
    {
        gl_program_id = gl_compile_shaders(vertex_shader_source, fragment_shader_source, err_handler);
        if (gl_program_id == 0u)
            return false;

        bool success = true;
        success &= gl_get_uniform_location(gl_program_id, "mat_proj", &gl_locations.mat_proj, err_handler);
        success &= gl_get_uniform_location(gl_program_id, "uni_color", &gl_locations.uni_color, err_handler);
        success &= gl_get_uniform_location(gl_program_id, "pt_size", &gl_locations.pt_size, err_handler);
        success &= gl_get_attrib_location(gl_program_id, "v_pos", &gl_locations.v_pos, err_handler);

        glGenVertexArrays(N_VAOS, &gl_vaos[0]);
        glGenBuffers(N_BUFFERS, &gl_buffers[0]);

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        glEnable(GL_PROGRAM_POINT_SIZE);

        return success;
    }

    void render_background(const shapes::BoundingBox2d<float>& bb, const lin::mat4f& mat_proj)
    {
        background_vertices = {
            bb.min().x, bb.min().y, 0.f,
            bb.min().x, bb.max().y, 0.f,
            bb.max().x, bb.min().y, 0.f,
            bb.max().x, bb.max().y, 0.f
        };

        glBindVertexArray(gl_vaos[0]);
        glBindBuffer(GL_ARRAY_BUFFER, gl_buffers[2]);
        glBufferData(GL_ARRAY_BUFFER, gl_size_in_bytes(background_vertices), static_cast<const void*>(background_vertices.data()), GL_STATIC_DRAW);
        glEnableVertexAttribArray(gl_locations.v_pos);
        glVertexAttribPointer(gl_locations.v_pos, 3, GL_FLOAT, /* normalized */ GL_FALSE, /* stride */ 0, GLoffsetf(0));

        // Run program
        glUseProgram(gl_program_id);
        glUniformMatrix4fv(static_cast<GLint>(gl_locations.mat_proj), 1, GL_TRUE, mat_proj.data());
        glUniform4fv(static_cast<GLint>(gl_locations.uni_color), 1, background_color.data());
        glUniform1f(static_cast<GLint>(gl_locations.pt_size), 1.f);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glUseProgram(0);
        glBindVertexArray(0);
    }

    void render_assets(const lin::mat4f& mat_proj)
    {
        if (draw_list.m_draw_calls.empty()) { return; }
        if (draw_list.m_buffer_version == 0) { return; }

        glBindVertexArray(gl_vaos[1]);
        if (draw_list_last_buffer_version != draw_list.m_buffer_version)
        {
            // Copy buffers
            glBindBuffer(GL_ARRAY_BUFFER, gl_buffers[0]);
            glBufferData(GL_ARRAY_BUFFER, gl_size_in_bytes(draw_list.m_vertices), static_cast<const void*>(draw_list.m_vertices.data()), GL_STATIC_DRAW);
            glEnableVertexAttribArray(gl_locations.v_pos);
            glVertexAttribPointer(gl_locations.v_pos, 3, GL_FLOAT, /* normalized */ GL_FALSE, /* stride */ 0, GLoffsetf(0));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_buffers[1]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, gl_size_in_bytes(draw_list.m_indices), static_cast<const void*>(draw_list.m_indices.data()), GL_STATIC_DRAW);
        }
        draw_list_last_buffer_version = draw_list.m_buffer_version;

        // Run program
        glUseProgram(gl_program_id);
        glUniformMatrix4fv(static_cast<GLint>(gl_locations.mat_proj), 1, GL_TRUE, mat_proj.data());
        for (const auto& draw_call : draw_list.m_draw_calls)
        {
            assert(draw_call.m_range.first <= draw_call.m_range.second);
            const auto count = static_cast<GLsizei>(draw_call.m_range.second - draw_call.m_range.first);
            glUniform4fv(static_cast<GLint>(gl_locations.uni_color), 1, draw_call.m_uniform_color.data());
            glUniform1f(static_cast<GLint>(gl_locations.pt_size), draw_call.m_uniform_point_size);
            glDrawElements(to_gl_draw_cmd(draw_call.m_cmd), count, GL_UNSIGNED_INT, GLoffsetui(draw_call.m_range.first));
        }
        glUseProgram(0);
        glBindVertexArray(0);
    }

    void render(const Canvas<float>& canvas, float window_height, Flag::type flags)
    {
        // Set OpenGL viewport
        // NB: The (0, 0) position in OpenGL is the bottom-left corner of the window, and the Y-axis is in the "up" direction.
        // For that reason we need to transform the y value of the bottom-left corner of our canvas.
        const ScreenPos canvas_bl(canvas.get_tl_corner().x, canvas.get_br_corner().y);
        const auto canvas_sz = canvas.get_size();
        glViewport(static_cast<GLint>(canvas_bl.x), static_cast<GLint>(window_height - canvas_bl.y), static_cast<GLsizei>(canvas_sz.x), static_cast<GLsizei>(canvas_sz.y));

        // Projection matrix
        const bool flip_y = flags & Flag::FlipYAxis;
        const auto bb = canvas.actual_bounding_box();
        const auto mat_proj = gl_orth_proj_mat(bb, flip_y);

        // Render background
        if (has_background) { render_background(bb, mat_proj); }

        // Render assets
        const bool draw_assets = !(flags & Flag::OnlyBackground);
        if (draw_assets) { render_assets(mat_proj); }
    }

    DrawList draw_list;
    DrawList::Version draw_list_last_buffer_version;
    GLuint gl_program_id;
    std::array<GLuint, N_VAOS> gl_vaos;
    std::array<GLuint, N_BUFFERS> gl_buffers;
    GLlocations gl_locations;
    bool has_background;
    std::array<float, 12> background_vertices;
    ColorData background_color;
    const stdutils::io::ErrorHandler* err_handler;
};

Draw2D::Draw2D(const stdutils::io::ErrorHandler* err_handler)
    : p_impl(std::make_unique<Impl>(err_handler))
{ }

Draw2D::~Draw2D() = default;

bool Draw2D::init()
{
    return p_impl->init();
}

DrawList& Draw2D::draw_list()
{
    return p_impl->draw_list;
}

void Draw2D::set_background_color(const ColorData& color)
{
    p_impl->has_background = true;
    p_impl->background_color = color;
}

void Draw2D::set_background_color(float r, float g, float b, float a)
{
    p_impl->has_background = true;
    p_impl->background_color = { r, g, b, a };
}

void Draw2D::reset_background_color()
{
    p_impl->has_background = false;
}

void Draw2D::render(const Canvas<float>& canvas, float window_height, Flag::type flags)
{
    p_impl->render(canvas, window_height, flags);
}

} // namespace renderer
