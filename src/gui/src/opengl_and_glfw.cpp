#include "opengl_and_glfw.h"

#include <cassert>
#include <sstream>

namespace
{

// Target OpenGL 3.3 for this project
constexpr int TARGET_OPENGL_MAJOR = 3;
constexpr int TARGET_OPENGL_MINOR = 3;
constexpr bool TARGET_OPENGL_CORE_PROFILE = true;       // 3.2+ only. Recommended.
constexpr const char* TARGET_GLSL_VERSION_STR = "#version 330 core";

stdutils::io::ErrorHandler s_glfw_err_handler;

void glfw_error_callback(int error, const char* description)
{
    if(!s_glfw_err_handler) { return; }
    std::stringstream out;
    out << "GLFW Error " << error << ": " << description;
    s_glfw_err_handler(stdutils::io::Severity::ERR, out.str());
}

} // Anonymous namespace

GLFWWindowContext::GLFWWindowContext(int width, int height, const std::string_view& title, const stdutils::io::ErrorHandler* err_handler)
    : m_window_ptr(nullptr)
    , m_glfw_init(false)
{
    static bool call_once = false;
    if (call_once)
        return;
    call_once = true;

    if (err_handler)
    {
        s_glfw_err_handler = *err_handler;
        glfwSetErrorCallback(glfw_error_callback);
    }
    if (!glfwInit())
    {
       if (err_handler) { (*err_handler)(stdutils::io::Severity::FATAL, "GLFW failed to initialize"); }
        return;
    }
    m_glfw_init = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, TARGET_OPENGL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, TARGET_OPENGL_MINOR);
    if constexpr (TARGET_OPENGL_CORE_PROFILE)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if constexpr (TARGET_OPENGL_CORE_PROFILE && TARGET_OPENGL_MAJOR >= 3)
    {
        // Forward Compatibility: Disable all the deprecated, but still in the core profile, functionalities
        // This is particularly important on MacOS.
        // See: https://www.khronos.org/opengl/wiki/OpenGL_Context#Forward_compatibility
        //      https://www.glfw.org/faq.html#41---how-do-i-create-an-opengl-30-context
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }
    m_window_ptr = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if (m_window_ptr == nullptr)
    {
        if (err_handler) { (*err_handler)(stdutils::io::Severity::FATAL, "GLFW failed to create the window"); }
        return;
    }
    glfwMakeContextCurrent(m_window_ptr);
}

GLFWWindowContext::~GLFWWindowContext()
{
    if(m_window_ptr)
        glfwDestroyWindow(m_window_ptr);
    m_window_ptr = nullptr;
    if (m_glfw_init)
        glfwTerminate();
}

bool load_opengl(const stdutils::io::ErrorHandler* err_handler)
{
    static bool called_once = false;
    if (called_once) { assert(0); return false; }
    called_once = true;

    const int gl3w_err = gl3wInit();
    if (gl3w_err != GL3W_OK)
    {
        if (err_handler)
        {
            std::stringstream out;
            out << "GL3W failed to initialize OpenGL (error code: " << gl3w_err << ")";
            (*err_handler)(stdutils::io::Severity::FATAL, out.str());
        }
        return false;
    }
    if (!gl3wIsSupported(TARGET_OPENGL_MAJOR, TARGET_OPENGL_MINOR))
    {
        if (err_handler)
        {
            std::stringstream out;
            out << "OpenGL " << TARGET_OPENGL_MAJOR << "." << TARGET_OPENGL_MINOR << " is not supported";
            (*err_handler)(stdutils::io::Severity::FATAL, out.str());
        }
        return false;
    }
    return true;
}

const char* glsl_version()
{
    return TARGET_GLSL_VERSION_STR;
}

#ifdef _WIN32
// On Windows, here is a trick to select the main GPU fo rthe application (instead of the integrated GPU)
// See Technical Note from NVidia: "Enabling High Performance Graphics Rendering on Optimus Systems"
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
