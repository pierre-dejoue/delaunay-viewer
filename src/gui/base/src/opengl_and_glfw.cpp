// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <gui/base/opengl_and_glfw.h>

#include <stdutils/macros.h>

#include <array>
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

// Caution: GL_DEBUG_OUTPUT is not supported at all on macOS
#ifndef SUPPORT_OPENGL_DEBUG_OUTPUT
#define SUPPORT_OPENGL_DEBUG_OUTPUT 0
#endif

#ifndef TRACE_GLFW_WINDOW_PROPERTIES
#ifndef NDEBUG
#define TRACE_GLFW_WINDOW_PROPERTIES 1
#endif
#endif

namespace {

// Target OpenGL 3.3 for this project.
// macOS: OpenGL 3.3 is supported starting with 10.9 Mavericks.
constexpr int TARGET_OPENGL_MAJOR = 3;
constexpr int TARGET_OPENGL_MINOR = 3;
constexpr bool TARGET_OPENGL_CORE_PROFILE = true;       // 3.2+ only. Recommended.
constexpr const char* TARGET_GLSL_VERSION_STR = "#version 330 core";

// OpengGL combined version number. It happens to match the GLSL version number ONLY IF >= 330.
constexpr int TARGET_OPENGL_VERSION_COMBINED = TARGET_OPENGL_MAJOR * 100 + TARGET_OPENGL_MINOR * 10 + 0;

// Forward Compatibility: Disable all the deprecated, yet still in the core profile, functionalities.
// This is particularly important on macOS that only supports fwd compatible OpenGL contexts
// See: https://www.khronos.org/opengl/wiki/OpenGL_Context#Forward_compatibility
//      https://www.glfw.org/faq.html#41---how-do-i-create-an-opengl-30-context
constexpr bool TARGET_OPENGL_FORWARD_COMPATIBILITY = TARGET_OPENGL_CORE_PROFILE && (TARGET_OPENGL_VERSION_COMBINED >= 300);

// Debug CONTEXT, for debug output. From the OpenGL doc:
// > Unless debug output is enabled, no messages will be generated, retrieved, or logged. It is enabled by using glEnable with the GL_DEBUG_OUTPUT enumerator.
// > In Debug Contexts, debug output starts enabled. In non-debug contexts, the OpenGL implementation may not generate messages even if debug output is enabled.
#ifndef NDEBUG
constexpr bool TARGET_OPENGL_DEBUG_CONTEXT = SUPPORT_OPENGL_DEBUG_OUTPUT;
#else
constexpr bool TARGET_OPENGL_DEBUG_CONTEXT = false;
#endif

// GLFW error handling function
stdutils::io::ErrorHandler s_glfw_err_handler;
void glfw_error_callback(int error, const char* description)
{
    if(!s_glfw_err_handler) { return; }
    std::stringstream out;
    out << "GLFW Error " << error << ": " << description;
    s_glfw_err_handler(stdutils::io::Severity::ERR, out.str());
}

std::string_view gl3w_error_code_as_string(int gl3w_err_code)
{
    assert(gl3w_err_code < 0);
    switch (gl3w_err_code)
    {
        case 0:
            return "GL3W_OK";

        case -1:
            return "GL3W_ERROR_INIT";

        case -2:
            return "GL3W_ERROR_LIBRARY_OPEN";

        case -3:
            return "GL3W_ERROR_OPENGL_VERSION";

        default:
            assert(0);
            return "GL3W_UNKNOWN";
    }
}

#if (GLFW_VERSION_COMBINED >= 3400)
std::string_view glfw_platform_as_string(int glfw_platform)
{
    switch (glfw_platform)
    {
        case GLFW_PLATFORM_WIN32:
            return "WIN32";

        case GLFW_PLATFORM_COCOA:
            return "COCOA";

        case GLFW_PLATFORM_WAYLAND:
            return "WAYLAND";

        case GLFW_PLATFORM_X11:
            return "X11";

        case GLFW_PLATFORM_NULL:
            return "NULL";

        default:
            return "UNKNOWN";
    }
}
#endif

struct ScrollEventSingleton
{
    ScrollEventSingleton()
        : m_window_ptr(nullptr)
        , m_scroll_event_callback()
        , m_chain_callback(nullptr)
    { }

    GLFWwindow*         m_window_ptr;
    ScrollEventCallback m_scroll_event_callback;
    GLFWscrollfun       m_chain_callback;
} g_scroll_event_singleton;

// Function with type GLFWscrollfun
void glfw_scroll_event_callback(GLFWwindow* window_ptr, double xoffset, double yoffset)
{
    if (g_scroll_event_singleton.m_window_ptr == window_ptr && g_scroll_event_singleton.m_scroll_event_callback)
    {
#if defined(__APPLE__)
        // GLFW's platform-specific code for macOS divides the offsets by 10 in the case of precise scrolling (for example in the case of a touchpad)
        // An excerpt of the code below for context:
        //
        // (NSEvent *)event
        //
        // double deltaX = [event scrollingDeltaX];
        // double deltaY = [event scrollingDeltaY];
        //
        // if ([event hasPreciseScrollingDeltas])
        // {
        //     deltaX *= 0.1;
        //     deltaY *= 0.1;
        // }
        //
        constexpr double offset_factor = 10.0;
#else
        constexpr double offset_factor = 1.0;
#endif
        g_scroll_event_singleton.m_scroll_event_callback(
            ScreenVect(
                static_cast<float>(offset_factor * xoffset),
                static_cast<float>(offset_factor * yoffset))
        );
    }
    if (g_scroll_event_singleton.m_chain_callback)
    {
        g_scroll_event_singleton.m_chain_callback(window_ptr, xoffset, yoffset);
    }
}

#if defined(GLFW_API_HAS_TRACKPAD_ZOOM)
struct ZoomEventSingleton
{
    ZoomEventSingleton()
        : m_window_ptr(nullptr)
        , m_zoom_event_callback()
        , m_chain_callback(nullptr)
    { }

    GLFWwindow*         m_window_ptr;
    ZoomEventCallback   m_zoom_event_callback;
    GLFWtrackpadzoomfun m_chain_callback;
} g_zoom_event_singleton;

// Function with type GLFWtrackpadzoomfun
void glfw_zoom_event_callback(GLFWwindow* window_ptr, double scale)
{
    if (g_zoom_event_singleton.m_window_ptr == window_ptr && g_zoom_event_singleton.m_zoom_event_callback)
    {
        g_zoom_event_singleton.m_zoom_event_callback(static_cast<float>(scale));
    }
    if (g_zoom_event_singleton.m_chain_callback)
    {
        g_zoom_event_singleton.m_chain_callback(window_ptr, scale);
    }
}
#endif

struct DroppedFileSingleton
{
    DroppedFileSingleton()
        : m_window_ptr(nullptr)
        , m_dropped_file_callback()
    { }

    GLFWwindow*         m_window_ptr;
    DroppedFileCallback m_dropped_file_callback;
} g_dropped_file_singleton;

void glfw_drop_callback(GLFWwindow* window_ptr, int path_count, const char** utf8_paths)
{
    // Only consider the first file if several are dropped simultaneously
    if (path_count > 0 && g_dropped_file_singleton.m_window_ptr == window_ptr && g_dropped_file_singleton.m_dropped_file_callback)
    {
        // TODO: In C++20 u8path gets deprecated, instead use std::u8string as argument to the path ctor
        fs::path path = fs::u8path(utf8_paths[0]);
        g_dropped_file_singleton.m_dropped_file_callback(std::move(path));
    }
}

} // namespace

GLFWWindowContext::GLFWWindowContext(int width, int height, const GLFWOptions& options, const stdutils::io::ErrorHandler* err_handler)
    : m_window_ptr(nullptr)
    , m_default_title("untitled")
    , m_glfw_init(false)
{
    static bool call_once = false;
    if (call_once)
        return;
    call_once = true;

    // Window title
    if (options.default_title.empty())
    {
        if (err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::WARN, "Window title is not specified"); }
    }
    else
    {
        m_default_title = options.default_title;
    }

    if (err_handler && *err_handler)
    {
        s_glfw_err_handler = *err_handler;
        glfwSetErrorCallback(glfw_error_callback);
    }

    if (!glfwInit())
    {
        if (err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::FATAL, "GLFW failed to initialize"); }
        return;
    }
    m_glfw_init = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, TARGET_OPENGL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, TARGET_OPENGL_MINOR);
    if constexpr (TARGET_OPENGL_CORE_PROFILE)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if constexpr (TARGET_OPENGL_FORWARD_COMPATIBILITY)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    if constexpr (TARGET_OPENGL_DEBUG_CONTEXT)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    if (options.maximize_window)
        glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
    if (options.framebuffer_msaa_samples > 0)
        glfwWindowHint(GLFW_SAMPLES, static_cast<int>(options.framebuffer_msaa_samples));
    assert(m_default_title.data());
    m_window_ptr = glfwCreateWindow(width, height, m_default_title.data(), nullptr, nullptr);
    if (m_window_ptr == nullptr)
    {
        if (err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::FATAL, "GLFW failed to create the window"); }
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

GLFWWindowContext::WindowStatus GLFWWindowContext::window_status() const
{
    WindowStatus result;
    assert(m_window_ptr);
    result.is_minimized = glfwGetWindowAttrib(m_window_ptr, GLFW_ICONIFIED);
    result.is_maximized = glfwGetWindowAttrib(m_window_ptr, GLFW_MAXIMIZED);
    return result;
}

std::pair<int, int> GLFWWindowContext::framebuffer_size() const
{
    std::pair<int, int> sz(0, 0);
    assert(m_window_ptr);
    glfwGetFramebufferSize(m_window_ptr, &sz.first, &sz.second);
    return sz;
}

std::pair<int, int> GLFWWindowContext::window_size() const
{
    std::pair<int, int> sz(0, 0);
    assert(m_window_ptr);
    glfwGetWindowSize(m_window_ptr, &sz.first, &sz.second);
    return sz;
}

void GLFWWindowContext::set_window_title(const char* title)
{
    assert(title);
    glfwSetWindowTitle(m_window_ptr, title);
}

void GLFWWindowContext::reset_window_title()
{
    glfwSetWindowTitle(m_window_ptr, m_default_title.data());
}

bool GLFWWindowContext::get_framebuffer_scale(float& scale) const
{
    assert(m_window_ptr);
    bool changed = false;
    if (scale <= 0.f)
    {
        scale = 1.f;
        changed = true;
    }

    std::pair<int, int> fb_sz(0, 0);
    glfwGetFramebufferSize(m_window_ptr, &fb_sz.first, &fb_sz.second);
    std::pair<int, int> window_sz(0, 0);
    glfwGetWindowSize(m_window_ptr, &window_sz.first, &window_sz.second);

    if (window_sz.first == 0 || window_sz.second == 0)
    {
        // Leave scale as-is
        return changed;
    }

    const std::pair<float, float> fb_scale(
        static_cast<float>(fb_sz.first)  / static_cast<float>(window_sz.first),
        static_cast<float>(fb_sz.second) / static_cast<float>(window_sz.second)
    );

#if TRACE_GLFW_WINDOW_PROPERTIES
    static std::pair<int, int> prev_fb_sz(0, 0);
    static std::pair<int, int> prev_window_sz(0, 0);
    bool any_change = false;
    if (prev_fb_sz != fb_sz)
    {
        prev_fb_sz = fb_sz;
        any_change = true;
    }
    if (prev_window_sz != window_sz)
    {
        prev_window_sz = window_sz;
        any_change = true;
    }
    if (any_change && s_glfw_err_handler)
    {
        std::stringstream out;
        out << "Framebuffer size: " << fb_sz.first << "x" << fb_sz.second;
        s_glfw_err_handler(stdutils::io::Severity::TRACE, out.str());
        out = std::stringstream();
        out << "Window size:      " << window_sz.first << "x" << window_sz.second;
        s_glfw_err_handler(stdutils::io::Severity::TRACE, out.str());
    }
#endif

    // If the scales are different on the X and Y axis, log an error and arbitrarily use scale.x
    static bool err_logged_once = false;
    if (s_glfw_err_handler && !err_logged_once && fb_scale.first != fb_scale.second)
    {
        std::stringstream out;
        out << "Different coordinate scales on the X and Y axis";
        out << "; Framebuffer: " << fb_sz.first << "x" << fb_sz.second;
        out << "; Window: " << window_sz.first << "x" << window_sz.second;
        s_glfw_err_handler(stdutils::io::Severity::ERR, out.str());
        err_logged_once = true;
    }

    // Set the scale
    if (scale != fb_scale.first)
    {
        scale = fb_scale.first;
        changed = true;
    }
    assert(scale > 0.f);

    return changed;
}

bool GLFWWindowContext::get_window_content_scale(float& scale) const
{
    assert(m_window_ptr);
    bool changed = false;
    if (scale <= 0.f)
    {
        scale = 1.f;
        changed = true;
    }

    std::pair<float, float> content_scale(0.f, 0.f);
    glfwGetWindowContentScale(m_window_ptr, &content_scale.first, &content_scale.second);

    if (content_scale.first <= 0.f || content_scale.second <= 0)
    {
        // Leave scale as-is
        return changed;
    }

#if TRACE_GLFW_WINDOW_PROPERTIES
    static std::pair<float, float> prev_content_scale(0.f, 0.f);
    bool any_change = false;
    if (prev_content_scale != content_scale)
    {
        prev_content_scale = content_scale;
        any_change = true;
    }
    if (any_change && s_glfw_err_handler)
    {
        std::stringstream out;
        out << "Window content scale: " << content_scale.first << ", " << content_scale.second;
        s_glfw_err_handler(stdutils::io::Severity::TRACE, out.str());
    }
#endif

    // Set the scale
    if (scale != content_scale.first)
    {
        scale = content_scale.first;
        changed = true;
    }
    assert(scale > 0.f);

    return changed;
}

void GLFWWindowContext::set_scroll_event_callback(ScrollEventCallback callback)
{
    g_scroll_event_singleton.m_window_ptr = m_window_ptr;
    g_scroll_event_singleton.m_scroll_event_callback = callback;
    g_scroll_event_singleton.m_chain_callback = glfwSetScrollCallback(m_window_ptr, glfw_scroll_event_callback);
}

void GLFWWindowContext::reset_scroll_event_callback()
{
    if (g_scroll_event_singleton.m_window_ptr)
    {
        glfwSetScrollCallback(m_window_ptr, g_scroll_event_singleton.m_chain_callback);
        g_scroll_event_singleton.m_chain_callback = nullptr;
        g_scroll_event_singleton.m_scroll_event_callback = ScrollEventCallback();
        g_scroll_event_singleton.m_window_ptr = m_window_ptr = nullptr;
    }
}

#if defined(GLFW_API_HAS_TRACKPAD_ZOOM)
void GLFWWindowContext::set_zoom_event_callback(ZoomEventCallback callback)
{
    g_zoom_event_singleton.m_window_ptr = m_window_ptr;
    g_zoom_event_singleton.m_zoom_event_callback = callback;
    g_zoom_event_singleton.m_chain_callback = glfwSetTrackpadZoomCallback(m_window_ptr, glfw_zoom_event_callback);
}
#endif

void GLFWWindowContext::set_dropped_file_callback(DroppedFileCallback callback)
{
    g_dropped_file_singleton.m_window_ptr = m_window_ptr;
    g_dropped_file_singleton.m_dropped_file_callback = callback;
    glfwSetDropCallback(m_window_ptr, glfw_drop_callback);
}

void GLFWWindowContext::reset_dropped_file_callback()
{
    if (g_dropped_file_singleton.m_window_ptr)
    {
        g_dropped_file_singleton.m_dropped_file_callback = DroppedFileCallback();
        g_dropped_file_singleton.m_window_ptr = nullptr;
    }
}

void GLFWWindowContext::glfw_version_info(std::ostream& out)
{
    out << "GLFW " << GLFW_VERSION_MAJOR << '.' << GLFW_VERSION_MINOR << '.' << GLFW_VERSION_REVISION;
#if (GLFW_VERSION_COMBINED >= 3400)
    out << " (" << glfw_platform_as_string(glfwGetPlatform()) << ")";
#endif
    out << '\n';
}

bool operator==(const GLFWvidmode& lhs, const GLFWvidmode& rhs)
{
    // Use case: Tested every frame. More often than not the video modes will be equal therefore we'll go through all the comparisons
    return (lhs.width       == rhs.width)
        &  (lhs.height      == rhs.height)
        &  (lhs.refreshRate == rhs.refreshRate)
        &  (lhs.redBits     == rhs.redBits)
        &  (lhs.greenBits   == rhs.greenBits)
        &  (lhs.blueBits    == rhs.blueBits);
}

std::ostream& operator<<(std::ostream& out, const GLFWvidmode& vid)
{
    return out << vid.width << 'x' << vid.height << "; " << vid.refreshRate << "Hz; R.G.B " << vid.redBits << '.' << vid.greenBits << '.' << vid.blueBits;
}

bool load_opengl(const stdutils::io::ErrorHandler* err_handler)
{
    static bool called_once = false;
    if (called_once) { assert(0); return false; }
    called_once = true;

    // Load OpenGL. Should be called once after an OpenGL context has been created.
    const int gl3w_err = gl3wInit();

    if (gl3w_err != GL3W_OK)
    {
        if (err_handler && *err_handler)
        {
            std::stringstream out;
            out << "GL3W failed to initialize OpenGL (error code: " << gl3w_err << " " << gl3w_error_code_as_string(gl3w_err) << ")";
            (*err_handler)(stdutils::io::Severity::FATAL, out.str());
        }
        return false;
    }
    if (!gl3wIsSupported(TARGET_OPENGL_MAJOR, TARGET_OPENGL_MINOR))
    {
        if (err_handler && *err_handler)
        {
            std::stringstream out;
            out << "OpenGL " << TARGET_OPENGL_MAJOR << "." << TARGET_OPENGL_MINOR << " is not supported";
            (*err_handler)(stdutils::io::Severity::FATAL, out.str());
        }
        return false;
    }
    return true;
}

void opengl_version_info(std::ostream& out)
{
    const auto* open_gl_version_str = glGetString(GL_VERSION);      // Will return NULL if there is no current OpenGL context!
    if (open_gl_version_str)
        out << "OpenGL Version: " << open_gl_version_str << '\n';
    const auto* open_gl_vendor_str = glGetString(GL_VENDOR);
    const auto* open_gl_renderer_str = glGetString(GL_RENDERER);
    if (open_gl_vendor_str && open_gl_renderer_str)
        out << "OpenGL Vendor: " << open_gl_vendor_str << "; Renderer: " << open_gl_renderer_str << '\n';
}

const char* glsl_version()
{
    return TARGET_GLSL_VERSION_STR;
}

std::string_view gl_error_str(GLenum error_enum)
{
    switch (error_enum)
    {
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_INVALID_INDEX:
            return "GL_INVALID_INDEX";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        default:
            return "UNKNOWN";
    }
}

std::string_view gl_debug_source_str(GLenum source_enum)
{
    // Source: https://www.khronos.org/opengl/wiki/Debug_Output
    switch (source_enum)
    {
        case GL_DEBUG_SOURCE_API:               // Calls to the OpenGL API
            return "OpenGL API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     // Calls to a window-system API
            return "Window System API";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:   // A compiler for a shading language
            return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:       // An application associated with OpenGL
            return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION:       // Generated by the user of this application
            return "Application User";
        case GL_DEBUG_SOURCE_OTHER:             // Some source that isn't one of these
            return "Other";
        default:
            return "UNKNOWN";
    }
}

std::string_view gl_debug_msg_type_str(GLenum msg_type_enum)
{
    // Source: https://www.khronos.org/opengl/wiki/Debug_Output
    switch (msg_type_enum)
    {

        case GL_DEBUG_TYPE_ERROR:                   // An error, typically from the API
            return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:     // Some behavior marked deprecated has been used
            return "Deprecated";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:      // Something has invoked undefined behavior
            return "Undefined Behavior";
        case GL_DEBUG_TYPE_PORTABILITY:             // Some functionality the user relies upon is not portable
            return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE:             // Code has triggered possible performance issues
            return "Performance";
        case GL_DEBUG_TYPE_MARKER:                  // Command stream annotation
            return "Marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:              // Group pushing
            return "Group Pushing";
        case GL_DEBUG_TYPE_POP_GROUP:               // Group popping
            return "Group Popping";
        case GL_DEBUG_TYPE_OTHER:                   // Some type that isn't one of these
            return "Other";
        default:
            return "UNKNOWN";
    }
}

std::string_view gl_debug_severity_str(GLenum sev_enum)
{
    // Source: https://www.khronos.org/opengl/wiki/Debug_Output
    switch (sev_enum)
    {
        case GL_DEBUG_SEVERITY_HIGH:            // All OpenGL Errors, shader compilation/linking errors, or highly-dangerous undefined behavior
            return "High";
        case GL_DEBUG_SEVERITY_MEDIUM:          // Major performance warnings, shader compilation/linking warnings, or the use of deprecated functionality
            return "Medium";
        case GL_DEBUG_SEVERITY_LOW:             // Redundant state change performance warning, or unimportant undefined behavior
            return "Low";
        case GL_DEBUG_SEVERITY_NOTIFICATION:    // Anything that isn't an error or performance issue.
            return "Notification";
        default:
            return "UNKNOWN";
    }
}

bool gl_errors(const char* context, const stdutils::io::ErrorHandler* err_handler)
{
    assert(context);
    GLenum err = GL_NO_ERROR;
    int count = 0;
    bool any_error = false;
    while ((err = glGetError()) != GL_NO_ERROR && count++ < 32)
    {
        if (!any_error && err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::ERR, std::string("OpenGL error occured during ") + context); }
        any_error = true;
        if (err_handler && *err_handler)
        {
            std::stringstream out;
            out << gl_error_str(err) << "("  << std::setbase(16) << std::showbase << err << ")";
            (*err_handler)(stdutils::io::Severity::ERR, out.str());
        }
    }
    return any_error;
}

bool gl_get_uniform_location(GLuint program, const GLchar *name, GLuint* out_location, const stdutils::io::ErrorHandler* err_handler)
{
    assert(name);
    assert(out_location);
    GLint id = glGetUniformLocation(program, name);
    if (id < 0)
    {
        if (err_handler && *err_handler)
        {
            std::stringstream out;
            out << "OpenGL: Uniform location [" << name << "] not found in the program";
             (*err_handler)(stdutils::io::Severity::ERR, out.str());
        }
        return false;
    }
    *out_location = static_cast<GLuint>(id);
    return true;
}

bool gl_get_attrib_location(GLuint program, const GLchar *name, GLuint* out_location, const stdutils::io::ErrorHandler* err_handler)
{
    assert(name);
    assert(out_location);
    GLint id = glGetAttribLocation(program, name);
    if (id < 0)
    {
        if (err_handler && *err_handler)
        {
            std::stringstream out;
            out << "OpenGL: Attribute location [" << name << "] not found in the program";
             (*err_handler)(stdutils::io::Severity::ERR, out.str());
        }
        return false;
    }
    *out_location = static_cast<GLuint>(id);
    return true;
}

namespace {

    // OpenGL debug output callaback
    // Ref: https://www.khronos.org/opengl/wiki/Debug_Output
    stdutils::io::ErrorHandler s_opengl_debug_output_err_handler;
    void gl_debug_output_callback(GLenum source, GLenum type, GLuint id, GLenum sev, GLsizei length,
                  const GLchar *msg, const void *user_param)
    {
        UNUSED(length);
        UNUSED(user_param);

        // Filter out bloat
#ifdef NDEBUG
        if (sev == GL_DEBUG_SEVERITY_NOTIFICATION) { return; }
#endif
        if (sev == GL_DEBUG_SEVERITY_NOTIFICATION && id == 131185) { return; }      // Buffer detailed info

        assert(s_opengl_debug_output_err_handler);
        std::stringstream out;
        out << "OpenGL Debug: Sev: " << gl_debug_severity_str(sev)
            << ", Source: " << gl_debug_source_str(source)
            << ", Type: " << gl_debug_msg_type_str(type)
            << ", Code: " << id
            << ", Msg: [" << msg << "]";
        stdutils::io::SeverityCode err_sev{0};
        switch (sev)
        {
            case GL_DEBUG_SEVERITY_HIGH:            // All OpenGL Errors, shader compilation/linking errors, or highly-dangerous undefined behavior
                err_sev = stdutils::io::Severity::ERR;
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:          // Major performance warnings, shader compilation/linking warnings, or the use of deprecated functionality
            case GL_DEBUG_SEVERITY_LOW:             // Redundant state change performance warning, or unimportant undefined behavior
            default:
                err_sev = stdutils::io::Severity::WARN;
                break;

            case GL_DEBUG_SEVERITY_NOTIFICATION:    // Anything that isn't an error or performance issue.
                err_sev = stdutils::io::Severity::TRACE;
                break;
        }
        s_opengl_debug_output_err_handler(err_sev, out.str());
    }

    enum class GlInfoLogType
    {
        Program,
        Shader
    };

    template <GlInfoLogType Type>
    void gl_object_trace_log(GLuint object_id, const stdutils::io::ErrorHandler* err_handler = nullptr)
    {
        if (!(err_handler  && *err_handler)) { return; }
        GLint str_length = 0;
        if constexpr (Type == GlInfoLogType::Program)
            glGetProgramiv(object_id, GL_INFO_LOG_LENGTH, &str_length);
        else
            glGetShaderiv(object_id, GL_INFO_LOG_LENGTH, &str_length);
        if (str_length ==  0) { return; }
        std::string log("GL ");
        if constexpr (Type == GlInfoLogType::Program)
            log.append("program");
        else
            log.append("shader");
        log.append(" info log [\n");
        const std::size_t log_begin = log.length();
        log.append(static_cast<std::size_t>(str_length), ' ');
        if constexpr (Type == GlInfoLogType::Program)
            glGetProgramInfoLog(object_id, str_length, nullptr, log.data() + log_begin);
        else
            glGetShaderInfoLog(object_id, str_length, nullptr, log.data() + log_begin);
        log.back() = ']';
        assert(err_handler && *err_handler);
        (*err_handler)(stdutils::io::Severity::TRACE, log);
    }

    bool check_shader_compilation(GLuint shader_id, const char* context, bool trace_log, const stdutils::io::ErrorHandler* err_handler = nullptr)
    {
        assert(context);
        bool success = false;
        const auto err_report = [err_handler, shader_id, context](const bool error, const std::string_view& msg) {
            if (err_handler && *err_handler)
            {
                std::stringstream out;
                out << "Compile " << context << " " << shader_id << ": " << msg;
                (*err_handler)(error ? stdutils::io::Severity::ERR : stdutils::io::Severity::TRACE, out.str());
            }
        };
        if (shader_id == 0) { err_report(true, "Shader id is zero"); return false; }
        GLint status = 0;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
        success = (status == GL_TRUE);
        if (trace_log || !success)
        {
            err_report(!success, success ? "success" : "failed");
            gl_object_trace_log<GlInfoLogType::Shader>(shader_id, err_handler);
        }
        return success;
    }

} // namespace

GLuint gl_compile_shaders(const char* vertex_shader, const char* fragment_shader, const stdutils::io::ErrorHandler* err_handler)
{
    std::array<const char*, 2> shader_strs;
    shader_strs[0] = TARGET_GLSL_VERSION_STR;
    constexpr bool trace_log = false;

    // Create the program
    const GLuint program_id = glCreateProgram();
    if (program_id == 0u)
    {
        if (err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "glCreateProgram() failed"); }
        return 0;
    }
    else if (err_handler  && *err_handler && trace_log)
    {
        std::stringstream out;
        out << "Create program " << program_id;
        (*err_handler)(stdutils::io::Severity::TRACE, out.str());
    }

    // Compile vertex shader
    const GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    if (vertex_shader_id == 0u)
    {
        if (err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "glCreateShader(GL_VERTEX_SHADER) failed"); }
        return 0;
    }
    shader_strs[1] = vertex_shader;
    glShaderSource(vertex_shader_id, 2, &shader_strs[0], nullptr);
    glCompileShader(vertex_shader_id);
    if (!check_shader_compilation(vertex_shader_id, "vertex shader", trace_log, err_handler)) { return 0; }

    // Compile fragment shader
    const GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragment_shader_id == 0u)
    {
        if (err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "glCreateShader(GL_FRAGMENT_SHADER) failed"); }
        return 0;
    }
    shader_strs[1] = fragment_shader;
    glShaderSource(fragment_shader_id, 2, &shader_strs[0], nullptr);
    glCompileShader(fragment_shader_id);
    if (!check_shader_compilation(fragment_shader_id, "fragment shader", trace_log, err_handler)) { return 0; }

    // Link the program
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    // Free shaders resources
    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Check the program
    GLint link_status;
    glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
    gl_object_trace_log<GlInfoLogType::Program>(program_id, err_handler);
    if (link_status != GL_TRUE)
    {
        if (err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "glLinkProgram() failed"); }
        return 0;
    }

    // In Debug builds, also validate the program
#ifndef NDEBUG
    glValidateProgram(program_id);
    GLint validation_status;
    glGetProgramiv(program_id, GL_LINK_STATUS, &validation_status);
    gl_object_trace_log<GlInfoLogType::Program>(program_id, err_handler);
    if (validation_status != GL_TRUE)
    {
        if (err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "GL program validation failed"); }
        return 0;
    }
#endif

    return program_id;
}

void gl_enable_debug(const stdutils::io::ErrorHandler& err_handler)
{
#if SUPPORT_OPENGL_DEBUG_OUTPUT
    if (!s_opengl_debug_output_err_handler) { s_opengl_debug_output_err_handler = err_handler; }
    glEnable(GL_DEBUG_OUTPUT);
#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    // Requires OpenGL 4.3 (will crash on macOS)
    glDebugMessageCallback(gl_debug_output_callback, nullptr);
    //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#else
    UNUSED(err_handler);
    assert(0);
#endif
}

lin::mat4f gl_orth_proj_mat(const shapes::BoundingBox3d<float>& screen_3d_bb, bool flip_y)
{
    const float& l = screen_3d_bb.min().x;
    const float& r = screen_3d_bb.max().x;
    const float& b = screen_3d_bb.min().y;
    const float& t = screen_3d_bb.max().y;
    const float& f = screen_3d_bb.min().z;              // far = min_z because the z axis is towards the viewer
    const float& n = screen_3d_bb.max().z;
    lin::mat4f proj = {
        2.f / (r -l), 0.f,           0.f,           -(r + l) / (r - l),
        0.f,          2.f / (t - b), 0.f,           -(t + b) / (t - b),
        0.f,          0.f,           2.f / (n - f), -(f + n) / (n - f),
        0.f,          0.f,           0.f,            1.f
    };
    if (flip_y)
    {
        proj[1][1] = -proj[1][1];
        proj[1][3] = -proj[1][3];
    }
    return proj;
}

lin::mat4f gl_orth_proj_mat(const shapes::BoundingBox2d<float>& screen_bb, bool flip_y, float n, float f)
{
    const auto z_range = std::minmax<float>(n, f);
    auto screen_3d_bb = shapes::BoundingBox3d<float>().add(screen_bb.min().x, screen_bb.min().y, z_range.first).add(screen_bb.max().x, screen_bb.max().y, z_range.second);
    lin::mat4f proj = gl_orth_proj_mat(screen_3d_bb, flip_y);
    return proj;
}

GLFWWindowContext create_glfw_window_load_opengl(int width, int height, const GLFWOptions& options, bool& any_fatal_error, unsigned int& back_framebuffer_id, const stdutils::io::ErrorHandler* err_handler)
{
    any_fatal_error = false;
    GLFWWindowContext glfw_context(width, height, options, err_handler);
    if (glfw_context.window() == nullptr)
        any_fatal_error = true;
    if (!load_opengl(err_handler))
        any_fatal_error = true;
    if constexpr (TARGET_OPENGL_DEBUG_CONTEXT)
    {
        if (!any_fatal_error && err_handler && *err_handler)
            gl_enable_debug(*err_handler);
    }

    // Vsync
    if (!any_fatal_error && options.enable_vsync)
        glfwSwapInterval(1);

    // Read the id of the back framebuffer (Usually this will be 0)
    if (!any_fatal_error)
    {
        GLint fb_id = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb_id);
        if (fb_id < 0)
        {
            if (err_handler && *err_handler) { (*err_handler)(stdutils::io::Severity::FATAL, "Negative back framebuffer id"); }
            any_fatal_error = true;
        }
        back_framebuffer_id = static_cast<unsigned int>(fb_id);
    }
    return glfw_context;
}

#ifdef _WIN32
// On Windows, here is a trick to select the main GPU for the application (instead of the integrated GPU)
// See Technical Note from NVidia: "Enabling High Performance Graphics Rendering on Optimus Systems"
// Alternatively we could rely on GLFW to do that for us with the GLFW_USE_HYBRID_HPG option (providing that lib is statically linked)
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
