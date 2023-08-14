#pragma once

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
