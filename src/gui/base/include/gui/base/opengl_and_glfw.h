// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

//
// Include this file to get access to the OpenGL API and the GLFW API, as well as some utilities.
//

#include <gui/abstract/canvas.h>
#include <lin/mat.h>
#include <shapes/bounding_box.h>
#include <stdutils/io.h>

#include <GL/gl3w.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// Prevent namespace pollution
#ifdef _WIN32
#undef min
#undef max
#endif

#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <ostream>
#include <string_view>
#include <utility>

// From Dear ImGui: imgui_impl_glfw.cpp
#define GLFW_VERSION_COMBINED (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 + GLFW_VERSION_REVISION)

struct GLFWOptions
{
    std::string_view default_title{};
    bool enable_vsync{true};
    bool maximize_window{false};
    unsigned int framebuffer_msaa_samples{0};         // 0 to disable multisampling
};

using ScrollEventCallback = std::function<void(ScreenVect)>;
using ZoomEventCallback = std::function<void(float)>;
using DroppedFileCallback = std::function<void(std::filesystem::path&&)>;

// Wrapper class for GLFW initialization and window
class GLFWWindowContext
{
public:
    struct WindowStatus
    {
        bool is_minimized{false};
        bool is_maximized{false};
    };

    GLFWWindowContext(int width, int height, const GLFWOptions& options, const stdutils::io::ErrorHandler* err_handler = nullptr);
    ~GLFWWindowContext();
    GLFWwindow* window() { return m_window_ptr; }
    WindowStatus window_status() const;
    std::pair<int, int> framebuffer_size() const;
    std::pair<int, int> window_size() const;

    // Window title. Reset will restore the default title set at the construction of the context.
    void set_window_title(const char* title);
    void reset_window_title();

    // Ratio between the framebuffer coordinates and the screen coordinates. Supposedly the same on the X and Y axis. Return true if the value changed.
    bool get_framebuffer_scale(float& scale) const;

    // DPI awareness
    bool get_window_content_scale(float& scale) const;

    // Set a callback for the scroll event coming from the mouse wheel or a touchpad
    void set_scroll_event_callback(ScrollEventCallback callback);
    void reset_scroll_event_callback();

#if defined(GLFW_API_HAS_TRACKPAD_ZOOM)
    static constexpr bool supports_trackpad_zoom_events() noexcept { return true; }

    // Set a callback for the zoom event coming from the touchpad
    void set_zoom_event_callback(ZoomEventCallback callback);
#else
    static constexpr bool supports_trackpad_zoom_events() noexcept { return false; }
    void set_zoom_event_callback(ZoomEventCallback) { assert(0); }
#endif

    // Set a callback for files being dragged and dropped onto the window.
    // If several files are dropped on the window, only the first one is sent to the callback.
    void set_dropped_file_callback(DroppedFileCallback callback);
    void reset_dropped_file_callback();

    static void glfw_version_info(std::ostream& out);

private:
    GLFWwindow*     m_window_ptr;
    std::string     m_default_title;
    bool            m_glfw_init;
};

// GLFW video mode utilities
bool operator==(const GLFWvidmode& lhs, const GLFWvidmode& rhs);
inline bool operator!=(const GLFWvidmode& lhs, const GLFWvidmode& rhs) { return !(lhs == rhs); }
std::ostream& operator<<(std::ostream& out, const GLFWvidmode& vid);

// Load OpenGL functions. Call only once.
bool load_opengl(const stdutils::io::ErrorHandler* err_handler = nullptr);

// OpenGL version info. Call only after an OpenGL context is setup!
void opengl_version_info(std::ostream& out);

// #version line to use in GLSL shaders
const char* glsl_version();

// Utility functions to translate OpenGL codes to readable strings
std::string_view gl_error_str(GLenum error_enum);
std::string_view gl_debug_source_str(GLenum source_enum);
std::string_view gl_debug_msg_type_str(GLenum msg_type_enum);
std::string_view gl_debug_severity_str(GLenum sev_enum);

// Check for OpenGL errors. Return true if at least an error was detected
bool gl_errors(const char* context, const stdutils::io::ErrorHandler* err_handler = nullptr);

// Wrapper functions with error handling (in the case no GLError is generated)
bool gl_get_uniform_location(GLuint program, const GLchar *name, GLuint* out_location, const stdutils::io::ErrorHandler* err_handler = nullptr);
bool gl_get_attrib_location(GLuint program, const GLchar *name, GLuint* out_location, const stdutils::io::ErrorHandler* err_handler = nullptr);

// Enable debug log
// Not supported on macOS
void gl_enable_debug(const stdutils::io::ErrorHandler& err_handler);

// Send the GLSL code without the version line
GLuint gl_compile_shaders(const char* vertex_shader, const char* fragment_shader, const stdutils::io::ErrorHandler* err_handler = nullptr);

// Projection matrix
lin::mat4f gl_orth_proj_mat(const shapes::BoundingBox3d<float>& screen_3d_bb, bool flip_y = false);
lin::mat4f gl_orth_proj_mat(const shapes::BoundingBox2d<float>& screen_bb, bool flip_y = false, float n = 1.f, float f = -1.f);

// Factory for the GLFW context that also load OpenGL and enable debugging
GLFWWindowContext create_glfw_window_load_opengl(int width, int height, const GLFWOptions& options, bool& any_fatal_error, unsigned int& back_framebuffer_id, const stdutils::io::ErrorHandler* err_handler = nullptr);

// Helpers
template <typename T>
class GLoffset
{
public:
    explicit GLoffset(const std::size_t& count) : m_offset_value(count * sizeof(T)) {}
    operator const void*() const { return reinterpret_cast<const void*>(m_offset_value); }
private:
    std::size_t m_offset_value;
};
using GLoffsetf = GLoffset<float>;
using GLoffseti = GLoffset<std::int32_t>;
using GLoffsetui = GLoffset<std::uint32_t>;

template <typename T, typename C>
GLsizeiptr gl_container_size_in_bytes(const C& cont)
{
    static_assert(std::is_same_v<typename C::value_type, T>);
    return static_cast<GLsizeiptr>(cont.size() * sizeof(T));
}

template <typename T, std::size_t N>
constexpr GLsizeiptr gl_container_size_in_bytes(const std::array<T, N>&)
{
    return static_cast<GLsizeiptr>(N * sizeof(T));
}
