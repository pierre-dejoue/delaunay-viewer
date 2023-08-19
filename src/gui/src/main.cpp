/*******************************************************************************
 * DELAUNAY VIEWER
 *
 * A tool to test and compare Delaunay triangulation libraries
 *
 * Copyright (c) 2023 Pierre DEJOUE
 ******************************************************************************/

#include "opengl_draw_list.h"
#include "settings.h"
#include "shape_control_window.h"
#include "shaders.h"
#include "version.h"

#include <dt/dt_impl.h>
#include <lin/mat.h>
#include <shapes/bounding_box.h>
#include <shapes/bounding_box_algos.h>
#include <shapes/io.h>
#include <svg/svg.h>
#include <stdutils/io.h>
#include <stdutils/macros.h>

// Order matters in this section
#include <imgui_wrap.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <pfd_wrap.h>
#include "opengl_and_glfw.h"

#include <array>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>


namespace
{

const ImColor WindowBackgroundColor_Classic(29, 75, 99, 255);
const ImColor WindowBackgroundColor_Dark(4, 8, 25, 255);
const ImColor WindowMainBackgroundColor_Classic(35, 92, 121, 255);
const ImColor WindowMainBackgroundColor_Dark(10, 25, 50, 255);

#define TEST_TRIANGLE 0
#if TEST_TRIANGLE

OpenGLDrawList opengl_test_triangle_static_init()
{
    static OpenGLDrawList triangle;
    triangle.m_vertices = {
        { -0.5f, -0.2887f, 0.f },
        { 0.5f, -0.2887f, 0.f },
        { 0.f,   0.5773f, 0.f }
    };
    triangle.m_indices = {
        0, 1, 2,            // GL_TRIANGLES
        0, 1, 1, 2, 2, 0    // GL_LINES
    };
    auto& filled_shape = triangle.m_draw_calls.emplace_back();
    filled_shape.m_uniform_color = { 1.0f, 0.f, 0.f, 1.f };
    filled_shape.m_range = { 0, 3 };
    filled_shape.m_cmd = DrawCmd::Triangles;
    auto& border = triangle.m_draw_calls.emplace_back();
    border.m_uniform_color = { 0.0f, 0.f, 1.f, 1.f };
    border.m_range = { 3, 9 };
    border.m_cmd = DrawCmd::Lines;
    triangle.m_buffer_version = 1;
    return triangle;
}

const OpenGLDrawList& opengl_test_triangle()
{
    static OpenGLDrawList triangle = opengl_test_triangle_static_init();
    return triangle;
}
#endif

void imgui_set_style(bool dark_mode)
{
    if (dark_mode)
    {
        ImGui::StyleColorsDark();
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = WindowBackgroundColor_Dark;
    }
    else
    {
        ImGui::StyleColorsClassic();
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = WindowBackgroundColor_Classic;
    }
}

void err_callback(stdutils::io::SeverityCode sev, std::string_view msg)
{
    std::cerr << stdutils::io::str_severity_code(sev) << ": " << msg << std::endl;
}

} // Anonymous namespace

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    // Window size
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    // Window title
    std::stringstream window_title;
    window_title << "Delaunay Viewer " << get_version_string();
    std::cout << window_title.str() << std::endl;

    // Setup GLFW window and OpenGL
    stdutils::io::ErrorHandler err_handler(err_callback);
    GLFWWindowContext glfw_context(WINDOW_WIDTH, WINDOW_HEIGHT, window_title.str(), &err_handler);
    if (glfw_context.window() == nullptr)
        return 1;
    glfwSwapInterval(1);
    if (!load_opengl(&err_handler))
        return 1;
    gl_enable_debug(err_handler);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    UNUSED(io);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(glfw_context.window(), true);
    ImGui_ImplOpenGL3_Init(glsl_version());

    // Print out version information
    std::cout << "Dear ImGui " << IMGUI_VERSION << std::endl;
    std::cout << "GLFW " << GLFW_VERSION_MAJOR << "." << GLFW_VERSION_MINOR << "." << GLFW_VERSION_REVISION << std::endl;
    std::cout << "OpenGL Version " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer " << glGetString(GL_RENDERER) << std::endl;

    // Style
    bool imgui_dark_mode = false;
    imgui_set_style(imgui_dark_mode);

    // Open settings window
    Settings settings;
    settings.open_window();

    // Register the Delaunay triangulation implementations
    {
        const bool registered_delaunay_impl = delaunay::register_all_implementations();
        assert(registered_delaunay_impl);
        if (!registered_delaunay_impl)
            std::cerr << "Error during Delaunay implementations' registration" << std::endl;
    }

    // GL program
    GLuint gl_program_id = gl_compile_shaders(vertex_shader_source, fragment_shader_source, &err_handler);
    if (gl_program_id == 0u)
    {
        return 1;
    }

    // Setup viewport rendering
    const GLuint mat_proj_location = gl_get_uniform_location(gl_program_id, "mat_proj", &err_handler);
    const GLuint uni_color_location = gl_get_uniform_location(gl_program_id, "uni_color", &err_handler);
    const GLuint v_pos_location = gl_get_attrib_location(gl_program_id, "v_pos", &err_handler);
    GLuint gl_vao;
    glGenVertexArrays(1, &gl_vao);
    glBindVertexArray(gl_vao);
    std::array<GLuint, 2> gl_buffers;
    glGenBuffers(2, &gl_buffers[0]);
    glBindVertexArray(0);
    static OpenGLDrawList::Version last_opengl_draw_list_version = 0;

    // Main loop
    std::unique_ptr<ShapeWindow> shape_window;
    while (!glfwWindowShouldClose(glfw_context.window()))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        //int iconified = glfwGetWindowAttrib(glfw_context.window(), GLFW_ICONIFIED);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main menu
        if (ImGui::BeginMainMenuBar())
        {
            const stdutils::io::ErrorHandler io_err_handler = [](stdutils::io::SeverityCode, std::string_view msg) { std::cerr << msg << std::endl; };
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open CDT"))
                {
                    const auto paths = pfd::open_file("Select a CDT file", "",
                        { "CDT file", "*.cdt", "All files", "*.*" }).result();
                    for (const auto& path : paths)
                    {
                        std::cout << "User selected CDT file " << path << std::endl;
                        if (shapes::io::cdt::peek_point_dimension(path, io_err_handler) == 2)
                        {
                            auto cdt_shapes = shapes::io::cdt::parse_2d_shapes_from_file(path, io_err_handler);
                            const std::string filename = std::filesystem::path(path).filename().string();
                            shapes::io::ShapeAggregate<double> shapes;
                            if (!cdt_shapes.point_cloud.vertices.empty())
                            {
                                shapes.emplace_back(std::move(cdt_shapes.point_cloud));
                            }
                            if (!cdt_shapes.edges.vertices.empty())
                            {
                                auto point_paths = shapes::extract_paths(cdt_shapes.edges);
                                for (auto& pp: point_paths) { shapes.emplace_back(std::move(pp)); }
                            }
                            // Ignore Triangles2d
                            if (!shapes.empty())
                                shape_window = std::make_unique<ShapeWindow>(std::move(shapes), filename);
                        }
                        else
                        {
                            io_err_handler(stdutils::io::Severity::ERR, "Only support 2D points");
                        }
                    }
                }
                if (ImGui::MenuItem("Open DAT"))
                {
                    const auto paths = pfd::open_file("Select a DAT file", "",
                        { "DAT file", "*.dat", "All files", "*.*" }).result();
                    for (const auto& path : paths)
                    {
                        std::cout << "User selected DAT file " << path << std::endl;
                        auto shapes = shapes::io::dat::parse_shapes_from_file(path, io_err_handler);
                        const std::string filename = std::filesystem::path(path).filename().string();
                        if (!shapes.empty())
                            shape_window = std::make_unique<ShapeWindow>(std::move(shapes), filename);
                    }
                }
                if (ImGui::MenuItem("Open SVG"))
                {
                    const auto paths = pfd::open_file("Select a SVG file", "",
                        { "SVG file", "*.svg" }).result();
                    for (const auto& path : paths)
                    {
                        std::cout << "User selected SVG file " << path << std::endl;
                        auto file_paths = svg::io::parse_svg_paths(path, io_err_handler);
                        std::cout << "Nb of point paths: " << file_paths.point_paths.size() << ". Nb of cubic bezier paths: " << file_paths.cubic_bezier_paths.size() << "." << std::endl;
                        const std::string filename = std::filesystem::path(path).filename().string();
                        std::vector<shapes::AllShapes<double>> shapes;
                        shapes.reserve(file_paths.point_paths.size() + file_paths.cubic_bezier_paths.size());
                        for (const auto& pp : file_paths.point_paths)
                            shapes.emplace_back(std::move(pp));
                        for (const auto& cbp : file_paths.cubic_bezier_paths)
                            shapes.emplace_back(std::move(cbp));
                        if (!shapes.empty())
                            shape_window = std::make_unique<ShapeWindow>(std::move(shapes), filename);
                    }
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Options"))
                {
                    if (ImGui::MenuItem("Dark Mode", "", &imgui_dark_mode))
                    {
                        imgui_set_style(imgui_dark_mode);
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "Alt+F4"))
                {
                    glfwSetWindowShouldClose(glfw_context.window(), 1);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Draw shapes
        if(shape_window)
        {
            bool can_be_erased = false;
            shape_window->visit(can_be_erased, settings);
            if (can_be_erased)
                shape_window.reset();
        }

        // Settings window (always ON)
        {
            bool can_be_erased = false;
            settings.visit_window(can_be_erased);
        }

#if DELAUNAY_VIEWER_IMGUI_DEMO_FLAG
        // Dear Imgui Demo
        ImGui::ShowDemoWindow();
#endif

        // Prepare ImGui rendering
        ImGui::Render();

        // OpenGL Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(glfw_context.window(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        const auto clear_color = static_cast<ImVec4>(imgui_dark_mode ? WindowMainBackgroundColor_Dark : WindowMainBackgroundColor_Classic);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);


#if TEST_TRIANGLE
        const auto& gl_draw_list = opengl_test_triangle();
#else
        const auto& gl_draw_list = shape_window ? shape_window->get_opengl_draw_list() : OpenGLDrawList::empty();
#endif

        // Viewport rendering
        if (!gl_draw_list.m_draw_calls.empty() && display_w > 0 && display_h > 0)
        {
            glBindVertexArray(gl_vao);
            if (last_opengl_draw_list_version != gl_draw_list.m_buffer_version)
            {
                // Copy buffers
                glBindBuffer(GL_ARRAY_BUFFER, gl_buffers[0]);
                glBufferData(GL_ARRAY_BUFFER, gl_size_in_bytes(gl_draw_list.m_vertices), static_cast<const void*>(gl_draw_list.m_vertices.data()), GL_STATIC_DRAW);
                glEnableVertexAttribArray(v_pos_location);
                glVertexAttribPointer(v_pos_location, 3, GL_FLOAT, /* normalized */ GL_FALSE, /* stride */ 0, GLoffsetf(0));
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_buffers[1]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, gl_size_in_bytes(gl_draw_list.m_indices), static_cast<const void*>(gl_draw_list.m_indices.data()), GL_STATIC_DRAW);
            }
            last_opengl_draw_list_version = gl_draw_list.m_buffer_version;

            // Render
            #if TEST_TRIANGLE
            const auto r = static_cast<float>(display_w) / static_cast<float>(display_h);
            const auto bb = shapes::BoundingBox2d<float>().add(-r, -1.f).add(+r, 1.f);
            #else
            const auto target_bb = shapes::cast<float, ShapeWindow::scalar>(shape_window->get_canvas_bounding_box());
            const auto canvas = Canvas(0.f, 0.f, static_cast<float>(display_w), static_cast<float>(display_h), target_bb);
            const auto bb = canvas.actual_bounding_box();
            #endif
            const auto mat_proj = gl_orth_proj_mat(bb, settings.get_general_settings()->flip_y);
            glUseProgram(gl_program_id);
            glUniformMatrix4fv(static_cast<GLint>(mat_proj_location), 1, GL_TRUE, mat_proj.data());
            for (const auto& draw_call : gl_draw_list.m_draw_calls)
            {
                assert(draw_call.m_range.first <= draw_call.m_range.second);
                const auto count = static_cast<GLsizei>(draw_call.m_range.second - draw_call.m_range.first);
                glUniform4fv(static_cast<GLint>(uni_color_location), 1, draw_call.m_uniform_color.data());
                glDrawElements(to_gl_draw_cmd(draw_call.m_cmd), count, GL_UNSIGNED_INT, GLoffsetui(draw_call.m_range.first));
            }
            glBindVertexArray(0);
            glUseProgram(0);
        }

        // Imgui rendering
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // End frame
        glfwSwapBuffers(glfw_context.window());
    } // while (!glfwWindowShouldClose(window))

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

