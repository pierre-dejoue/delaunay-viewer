#pragma once

//
// Include this file to get access to the OpenGL API and the GLFW API, as well as some utilities.
//

#include <stdutils/io.h>

#include <GL/gl3w.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string_view>


// Wrapper class for GLFW initialization and window
class GLFWWindowContext
{
public:
    GLFWWindowContext(int width, int height, const std::string_view& title, const stdutils::io::ErrorHandler* err_handler = nullptr);
    ~GLFWWindowContext();
    GLFWwindow* window() { return m_window_ptr; }

private:
    GLFWwindow*     m_window_ptr;
    bool            m_glfw_init;
};

// Load OpenGL functions. Call only once.
bool load_opengl(const stdutils::io::ErrorHandler* err_handler = nullptr);

// #version line to use in GLSL shaders
const char* glsl_version();

// Utility functions to translate OpenGL codes to readable strings
std::string_view gl_error_str(GLenum error_enum);
std::string_view gl_debug_source_str(GLenum source_enum);
std::string_view gl_debug_msg_type_str(GLenum msg_type_enum);
std::string_view gl_debug_severity_str(GLenum sev_enum);

// Check for OpenGL errors. Return true if at least an error was detected
bool gl_errors(const char* context, const stdutils::io::ErrorHandler* err_handler = nullptr);

// Enable debug log
void gl_enable_debug(const stdutils::io::ErrorHandler& err_handler);

// Send the GLSL code without the version line
GLuint gl_compile_shaders(const char* vertex_shader, const char* fragment_shader, const stdutils::io::ErrorHandler* err_handler = nullptr);

// Helpers
using GLstride = GLsizei;
class GLoffset
{
public:
    explicit GLoffset(const std::size_t& offset_value) : m_offset_value(offset_value) {}
    operator const void*() const { return reinterpret_cast<const void*>(m_offset_value); }
private:
    std::size_t m_offset_value;
};
template <typename T>
GLsizeiptr gl_size_in_bytes(const std::vector<T>& vect) { return static_cast<GLsizeiptr>(vect.size() * sizeof(T)); }
template <typename T, std::size_t N>
constexpr GLsizeiptr gl_size_in_bytes(const std::array<T, N>&) { return static_cast<GLsizeiptr>(N * sizeof(T)); }
