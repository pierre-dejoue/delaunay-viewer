// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <base/opengl_and_glfw.h>

#include <stdutils/macros.h>

#include <array>
#include <algorithm>
#include <cassert>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

#ifndef SUPPORT_OPENGL_DEBUG_OUTPUT
#define SUPPORT_OPENGL_DEBUG_OUTPUT 0
#endif

namespace {

// Additional trace logs during GLFW initialization
constexpr bool TRACE_GLFW_WINDOW_PROPERTIES = false;

// Target OpenGL 3.3 for this project
constexpr int TARGET_OPENGL_MAJOR = 3;
constexpr int TARGET_OPENGL_MINOR = 3;
constexpr bool TARGET_OPENGL_CORE_PROFILE = true;       // 3.2+ only. Recommended.
constexpr const char* TARGET_GLSL_VERSION_STR = "#version 330 core";

// Forward Compatibility: Disable all the deprecated, yet still in the core profile, functionalities.
// This is particularly important on macOS.
// See: https://www.khronos.org/opengl/wiki/OpenGL_Context#Forward_compatibility
//      https://www.glfw.org/faq.html#41---how-do-i-create-an-opengl-30-context
constexpr bool TARGET_OPENGL_FORWARD_COMPATIBILITY = TARGET_OPENGL_CORE_PROFILE && TARGET_OPENGL_MAJOR >= 3;

// Debug CONTEXT, for debug output. From the OpenGL doc:
// > Unless debug output is enabled, no messages will be generated, retrieved, or logged. It is enabled by using glEnable with the GL_DEBUG_OUTPUT enumerator.
// > In Debug Contexts, debug output starts enabled. In non-debug contexts, the OpenGL implementation may not generate messages even if debug output is enabled.
#ifndef NDEBUG
constexpr bool TARGET_OPENGL_DEBUG_CONTEXT = true;
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

} // namespace

GLFWWindowContext::GLFWWindowContext(int width, int height, const GLFWOptions& options, const stdutils::io::ErrorHandler* err_handler)
    : m_window_ptr(nullptr)
    , m_glfw_init(false)
{
    static bool call_once = false;
    if (call_once)
        return;
    call_once = true;

    // Window title
    if (options.title.empty() && err_handler) { (*err_handler)(stdutils::io::Severity::WARN, "Window title is not specified"); }
    const std::string_view title = options.title.empty() ? "untitled" : options.title;

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
    if constexpr (TARGET_OPENGL_FORWARD_COMPATIBILITY)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    if constexpr (TARGET_OPENGL_DEBUG_CONTEXT)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    if (options.framebuffer_msaa_samples > 0)
    {
        glfwWindowHint(GLFW_SAMPLES, static_cast<int>(options.framebuffer_msaa_samples));
    }
    assert(title.data());
    m_window_ptr = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if (m_window_ptr == nullptr)
    {
        if (err_handler) { (*err_handler)(stdutils::io::Severity::FATAL, "GLFW failed to create the window"); }
        return;
    }
    glfwMakeContextCurrent(m_window_ptr);
    glfwSwapInterval(1);
}

GLFWWindowContext::~GLFWWindowContext()
{
    if(m_window_ptr)
        glfwDestroyWindow(m_window_ptr);
    m_window_ptr = nullptr;
    if (m_glfw_init)
        glfwTerminate();
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

float GLFWWindowContext::get_framebuffer_scale() const
{
    float scale{1.f};
    assert(m_window_ptr);

    std::pair<int, int> fb_sz(0, 0);
    glfwGetFramebufferSize(m_window_ptr, &fb_sz.first, &fb_sz.second);
    std::pair<int, int> window_sz(0, 0);
    glfwGetWindowSize(m_window_ptr, &window_sz.first, &window_sz.second);

    if (window_sz.first == 0 || window_sz.second == 0)
        return scale;

    const std::pair<float, float> fb_scale(
        static_cast<float>(fb_sz.first)  / static_cast<float>(window_sz.first),
        static_cast<float>(fb_sz.second) / static_cast<float>(window_sz.second)
    );

    // Trace the window properties
    if constexpr (TRACE_GLFW_WINDOW_PROPERTIES)
    {
        if (s_glfw_err_handler)
        {
            std::pair<float, float> content_scale(0.f, 0.f);
            glfwGetWindowContentScale(m_window_ptr, &content_scale.first, &content_scale.second);
            std::stringstream out;
            out << "Framebuffer size: " << fb_sz.first << "x" << fb_sz.second;
            s_glfw_err_handler(stdutils::io::Severity::TRACE, out.str());
            out = std::stringstream();
            out << "Window size:      " << window_sz.first << "x" << window_sz.second;
            s_glfw_err_handler(stdutils::io::Severity::TRACE, out.str());
            out = std::stringstream();
            out << "Window content scale: " << content_scale.first << ", " << content_scale.second;
            s_glfw_err_handler(stdutils::io::Severity::TRACE, out.str());
        }
    }

    // If the scales are different on the X and Y axis, log an error and arbitrarily use scale.x
    if (fb_scale.first != fb_scale.second && s_glfw_err_handler)
    {
        std::stringstream out;
        out << "Different coordinate scales on the X and Y axis";
        out << "; Framebuffer: " << fb_sz.first << "x" << fb_sz.second;
        out << "; Window: " << window_sz.first << "x" << window_sz.second;
        s_glfw_err_handler(stdutils::io::Severity::ERR, out.str());
    }
    scale = fb_scale.first;

    assert(scale > 0.f);
    return scale;
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
        if (!any_error && err_handler) { (*err_handler)(stdutils::io::Severity::ERR, std::string("OpenGL error occured during ") + context); }
        any_error = true;
        if (err_handler)
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
        if (err_handler)
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
        if (err_handler)
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

    // OpenGL debug output callaback;
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
        out << "Open GL Debug: Sev: " << gl_debug_severity_str(sev)
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
        if (err_handler == nullptr) { return; }
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
        (*err_handler)(stdutils::io::Severity::TRACE, log);
    }

    bool check_shader_compilation(GLuint shader_id, const char* context, bool trace_log, const stdutils::io::ErrorHandler* err_handler = nullptr)
    {
        assert(context);
        bool success = false;
        const auto err_report = [err_handler, shader_id, context](const bool error, const std::string_view& msg) {
            if (err_handler)
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
        if (err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "glCreateProgram() failed"); }
        return 0;
    }
    else if (err_handler && trace_log)
    {
        std::stringstream out;
        out << "Create program " << program_id;
        (*err_handler)(stdutils::io::Severity::TRACE, out.str());
    }

    // Compile vertex shader
    const GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    if (vertex_shader_id == 0u)
    {
        if (err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "glCreateShader(GL_VERTEX_SHADER) failed"); }
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
        if (err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "glCreateShader(GL_FRAGMENT_SHADER) failed"); }
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
        if (err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "glLinkProgram() failed"); }
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
        if (err_handler) { (*err_handler)(stdutils::io::Severity::ERR, "GL program validation failed"); }
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

lin::mat4f gl_orth_proj_mat(const shapes::BoundingBox3d<float>& bb, bool flip_y)
{
    const float& l = bb.min().x;
    const float& r = bb.max().x;
    const float& b = bb.min().y;
    const float& t = bb.max().y;
    const float& f = bb.min().z;            // Because the z axis is towards the viewer
    const float& n = bb.max().z;
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

lin::mat4f gl_orth_proj_mat(const shapes::BoundingBox2d<float>& bb, bool flip_y, float n, float f)
{
    const auto z_range = std::minmax<float>(n, f);
    auto bb3d = shapes::BoundingBox3d<float>().add(bb.min().x, bb.min().y, z_range.first).add(bb.max().x, bb.max().y, z_range.second);
    lin::mat4f proj = gl_orth_proj_mat(bb3d, flip_y);
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
#if SUPPORT_OPENGL_DEBUG_OUTPUT
    if (err_handler)
        gl_enable_debug(*err_handler);
#endif
    // Read the id of the back framebuffer (Usually this will be 0)
    {
        GLint fb_id = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb_id);
        if (fb_id < 0)
        {
            if (err_handler) { (*err_handler)(stdutils::io::Severity::FATAL, "Negative back framebuffer id"); }
            any_fatal_error = true;
        }
        back_framebuffer_id = static_cast<unsigned int>(fb_id);
    }
    return glfw_context;
}

#ifdef _WIN32
// On Windows, here is a trick to select the main GPU for the application (instead of the integrated GPU)
// See Technical Note from NVidia: "Enabling High Performance Graphics Rendering on Optimus Systems"
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
