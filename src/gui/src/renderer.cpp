// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "renderer.h"

#include <gui/base/opengl_and_glfw.h>
#include <lin/mat.h>
#include <stdutils/enum.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <type_traits>

namespace renderer {

namespace {

struct VertexShaderSource
{
    static const char* main;
};

// The GLSL version number is added by our driver
const char* VertexShaderSource::main = R"SRC(

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

struct FragmentShaderSource
{
    static const char* main;
};

// The GLSL version number is added by our driver
const char* FragmentShaderSource::main = R"SRC(

in vec4 color;
layout (location = 0) out vec4 out_color;

void main()
{
    out_color = color;
}

)SRC";

const std::array<GLenum, stdutils::enum_size<DrawCmd>()> lookup_gl_draw_cmd {
    /* DrawCmd::Point */                GL_POINTS,
    /* DrawCmd::Lines */                GL_LINES,
    /* DrawCmd::Triangles */            GL_TRIANGLES
};

} // namespace

DrawList::DrawCall::DrawCall()
    : m_range(0, 0)
    , m_uniform_color({1.f, 0.f, 0.f, 1.f})
    , m_uniform_point_size(1.f)
    , m_cmd(renderer::DrawCmd::Lines)
{}


void DrawList::clear_all()
{
    // Clear draw calls
    m_draw_calls.clear();

    // Clear the buffers
    m_vertices.clear();
    m_indices.clear();

    // Increment buffer version number
    m_buffer_version++;
}

void DrawList::clear_draw_calls()
{
    // Clear the draw calls
    assert(m_buffer_version > 0);
    m_draw_calls.clear();

    // Reset the buffers indices but keep the data
    m_vertices.index_reset();
    m_indices.index_reset();
    assert(m_vertices.is_locked());
    assert(m_indices.is_locked());
}

void stable_sort_draw_commands(DrawList& draw_list)
{
    using T = std::underlying_type_t<DrawCmd>;
    std::stable_sort(std::begin(draw_list.m_draw_calls), std::end(draw_list.m_draw_calls), [](const auto& lhs, const auto& rhs) { return static_cast<T>(lhs.m_cmd) > static_cast<T>(rhs.m_cmd); });
}

// OpenGL Implementation
struct Draw2D::Impl
{
    static inline constexpr unsigned int N_VAOS = 2u;
    static inline constexpr unsigned int N_BUFFERS = 3u;

    struct GLLocations
    {
        GLuint mat_proj{0u};
        GLuint uni_color{0u};
        GLuint pt_size{0u};
        GLuint v_pos{0u};
    };

    struct Background
    {
        std::array<float, 12> corner_vertices{};
        ColorData             color{COLOR_DATA_BLACK};
    };

    Impl(const Settings& settings, const stdutils::io::ErrorHandler* err_handler);
    ~Impl();

    bool initialize_pipeline(const Settings& settings);
    bool init_framebuffer(int width, int height);
    void clear_framebuffer(ColorData clear_color);
    void set_opengl_viewport(const Canvas<float>& canvas);
    void update_corner_vertices(const Canvas<float>& canvas);
    void update_assets_buffers();
    void render_background();
    void render_assets();
    void render(const Canvas<float>& viewport_canvas);
    void render_viewport_background(const Canvas<float>& viewport_canvas);

    bool initialized;
    DrawList draw_list;
    DrawList::Version draw_list_last_buffer_version;
    struct {
        GLuint main{0u};
    } gl_program_ids;
    struct {
        GLLocations main{};
    } gl_locations;
    GLuint gl_back_framebuffer_id;
    std::pair<int, int> framebuffer_size;
    std::array<GLuint, N_VAOS> gl_vaos;
    std::array<GLuint, N_BUFFERS> gl_buffers;
    lin::mat4f mat_proj;
    Background background;
    const stdutils::io::ErrorHandler* err_handler;
};

Draw2D::Impl::Impl(const Settings& settings, const stdutils::io::ErrorHandler* err_handler)
    : initialized{false}
    , draw_list()
    , draw_list_last_buffer_version{0u}
    , gl_program_ids{}
    , gl_locations{}
    , gl_back_framebuffer_id{settings.back_framebuffer_id}
    , framebuffer_size(0, 0)
    , gl_vaos()
    , gl_buffers()
    , mat_proj(lin::mat4f::identity())
    , background{}
    , err_handler(err_handler)
{
    // Declare the programs
    bool success = true;
    {
        GLuint& program_id = gl_program_ids.main;
        GLLocations& locations = gl_locations.main;
        program_id = gl_compile_shaders(VertexShaderSource::main, FragmentShaderSource::main, err_handler);
        if (program_id == 0u)
            return;

        success &= gl_get_uniform_location(program_id, "mat_proj",  &locations.mat_proj,    err_handler);
        success &= gl_get_uniform_location(program_id, "uni_color", &locations.uni_color,   err_handler);
        success &= gl_get_uniform_location(program_id, "pt_size",   &locations.pt_size,     err_handler);
        success &= gl_get_attrib_location (program_id, "v_pos",     &locations.v_pos,       err_handler);
    }

    success &= initialize_pipeline(settings);

    initialized = success;
}

Draw2D::Impl::~Impl()
{
    glDeleteVertexArrays(N_VAOS, &gl_vaos[0]);
    glDeleteBuffers(N_BUFFERS, &gl_buffers[0]);
    if (gl_program_ids.main != 0u) { glDeleteProgram(gl_program_ids.main); }
}

bool Draw2D::Impl::initialize_pipeline(const Settings& settings)
{
    // Pipeline general configuration
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_MULTISAMPLE);
    if (settings.line_smooth)
        glEnable(GL_LINE_SMOOTH);
    else
        glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Buffers
    // VBO 0: background corner vertices
    // VBO 1: assets vertices
    // VBO 2: assets indices
    glGenBuffers(N_BUFFERS, &gl_buffers[0]);

    // Vertex Arrays
    glGenVertexArrays(N_VAOS, &gl_vaos[0]);

    // VAO 0: background
    glBindVertexArray(gl_vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, gl_buffers[0]);
    glEnableVertexAttribArray(gl_locations.main.v_pos);
    glVertexAttribPointer(gl_locations.main.v_pos, 3, GL_FLOAT, /* normalized */ GL_FALSE, /* stride */ 0, GLoffsetf(0));

    // VAO 1: main renderer
    glBindVertexArray(gl_vaos[1]);
    glBindBuffer(GL_ARRAY_BUFFER, gl_buffers[1]);
    glEnableVertexAttribArray(gl_locations.main.v_pos);
    glVertexAttribPointer(gl_locations.main.v_pos, 3, GL_FLOAT, /* normalized */ GL_FALSE, /* stride */ 0, GLoffsetf{0});
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_buffers[2]);

    glBindVertexArray(0);

    return true;
}

bool Draw2D::Impl::init_framebuffer(int width, int height)
{
    assert(initialized);
    if (width <= 0 || height <= 0)
        return true;    // Minimized window. Do nothing.

    framebuffer_size = std::pair<int, int>(width, height);
    return true;
}

void Draw2D::Impl::clear_framebuffer(ColorData clear_color)
{
    assert(initialized);
    glViewport(0, 0, framebuffer_size.first, framebuffer_size.second);
    glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);           // No depth buffer to clear
}

void Draw2D::Impl::set_opengl_viewport(const Canvas<float>& canvas)
{
    assert(initialized);
    // Set the viewport (coordinates transformation from clip space to window space)
    // NB: The (0, 0) position in OpenGL is the bottom-left corner of the window, and the Y-axis is in the "up" direction.
    // For that reason we need to transform the y value of the bottom-left corner of our canvas.
    const ScreenPos canvas_bl(canvas.get_tl_corner().x, canvas.get_br_corner().y);
    const auto canvas_sz = canvas.get_size();
    const float window_height = static_cast<float>(framebuffer_size.second);
    glViewport(static_cast<GLint>(canvas_bl.x), static_cast<GLint>(window_height - canvas_bl.y), static_cast<GLsizei>(canvas_sz.x), static_cast<GLsizei>(canvas_sz.y));
}

void Draw2D::Impl::update_corner_vertices(const Canvas<float>& canvas) {
    assert(initialized);
    const auto bb = canvas.actual_bounding_box();
    background.corner_vertices = {
        bb.min().x, bb.min().y, 0.f,
        bb.min().x, bb.max().y, 0.f,
        bb.max().x, bb.min().y, 0.f,
        bb.max().x, bb.max().y, 0.f
    };
    glBindBuffer(GL_ARRAY_BUFFER, gl_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, gl_container_size_in_bytes(background.corner_vertices), static_cast<const void*>(background.corner_vertices.data()), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Draw2D::Impl::update_assets_buffers()
{
    assert(initialized);
    assert(draw_list_last_buffer_version <= draw_list.buffer_version());
    if (draw_list_last_buffer_version != draw_list.buffer_version())
    {
        assert(draw_list.m_vertices.is_locked());
        assert(draw_list.m_indices.is_locked());
        glBindBuffer(GL_ARRAY_BUFFER, gl_buffers[1]);
        glBufferData(GL_ARRAY_BUFFER, gl_container_size_in_bytes<DrawList::VertexData>(draw_list.m_vertices), static_cast<const void*>(draw_list.m_vertices.data()), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_buffers[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, gl_container_size_in_bytes<DrawList::HWindex>(draw_list.m_indices), static_cast<const void*>(draw_list.m_indices.data()), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    draw_list_last_buffer_version = draw_list.buffer_version();
}

void Draw2D::Impl::render_background()
{
    assert(initialized);
    glBindVertexArray(gl_vaos[0]);
    glUseProgram(gl_program_ids.main);
    glUniformMatrix4fv(static_cast<GLint>(gl_locations.main.mat_proj), 1, GL_TRUE, mat_proj.data());
    glUniform4fv(static_cast<GLint>(gl_locations.main.uni_color), 1, background.color.data());
    glUniform1f(static_cast<GLint>(gl_locations.main.pt_size), 1.f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glUseProgram(0);
    glBindVertexArray(0);
}

void Draw2D::Impl::render_assets()
{
    assert(initialized);
    if (draw_list.m_draw_calls.empty()) { return; }
    if (draw_list.buffer_version() == 0) { return; }

    // Run main program
    glBindVertexArray(gl_vaos[1]);
    glUseProgram(gl_program_ids.main);
    glUniformMatrix4fv(static_cast<GLint>(gl_locations.main.mat_proj), 1, GL_TRUE, mat_proj.data());
    for (const auto& draw_call : draw_list.m_draw_calls)
    {
        assert(draw_call.m_range.first <= draw_call.m_range.second);
        const auto count = static_cast<GLsizei>(draw_call.m_range.second - draw_call.m_range.first);
        glUniform4fv(static_cast<GLint>(gl_locations.main.uni_color), 1, draw_call.m_uniform_color.data());
        glUniform1f(static_cast<GLint>(gl_locations.main.pt_size), draw_call.m_uniform_point_size);
        const auto draw_cmd = static_cast<std::size_t>(draw_call.m_cmd);                                                assert(draw_cmd < stdutils::enum_size<DrawCmd>());
        glDrawElements(lookup_gl_draw_cmd[draw_cmd], count, GL_UNSIGNED_INT, GLoffsetui(draw_call.m_range.first));
    }
    glUseProgram(0);
    glBindVertexArray(0);
}

void Draw2D::Impl::render(const Canvas<float>& viewport_canvas)
{
    assert(initialized);
    if (framebuffer_size.first == 0 || framebuffer_size.second == 0)
        return;

    // Projection matrix
    const bool flip_y = viewport_canvas.get_flip_y();
    const auto bb = viewport_canvas.actual_bounding_box();
    mat_proj = gl_orth_proj_mat(bb, flip_y);

    // Set OpenGL viewport
    set_opengl_viewport(viewport_canvas);

    // Vertex buffers
    update_assets_buffers();

    // Render assets
    render_assets();
}

void Draw2D::Impl::render_viewport_background(const Canvas<float>& viewport_canvas)
{
    assert(initialized);
    if (framebuffer_size.first == 0 || framebuffer_size.second == 0)
        return;

    // Projection matrix
    const auto bb = viewport_canvas.actual_bounding_box();
    mat_proj = gl_orth_proj_mat(bb);

    // Set OpenGL viewport
    set_opengl_viewport(viewport_canvas);

    // Vertex buffers
    update_corner_vertices(viewport_canvas);

    // Render background
    render_background();
}

Draw2D::Draw2D(const Settings& settings, const stdutils::io::ErrorHandler* err_handler)
    : p_impl(std::make_unique<Impl>(settings, err_handler))
{ }

Draw2D::~Draw2D() = default;

Draw2D::Draw2D(Draw2D&&) noexcept = default;

Draw2D& Draw2D::operator=(Draw2D&&) noexcept = default;

bool Draw2D::initialized() const
{
    return p_impl->initialized;
}

bool Draw2D::init_framebuffer(int width, int height)
{
    return p_impl->init_framebuffer(width, height);
}

void Draw2D::clear_framebuffer(ColorData clear_color)
{
    p_impl->clear_framebuffer(clear_color);
}

void Draw2D::set_viewport_background_color(const ColorData& color)
{
    p_impl->background.color = color;
}

void Draw2D::set_viewport_background_color(float r, float g, float b, float a)
{
    p_impl->background.color = ColorData{ r, g, b, a };
}

DrawList& Draw2D::draw_list()
{
    return p_impl->draw_list;
}

void Draw2D::render(const Canvas<float>& viewport_canvas)
{
    p_impl->render(viewport_canvas);
}

void Draw2D::render_viewport_background(const Canvas<float>& viewport_canvas)
{
    p_impl->render_viewport_background(viewport_canvas);
}

} // namespace renderer
