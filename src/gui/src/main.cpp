/*******************************************************************************
 * DELAUNAY VIEWER
 *
 * A tool to test and compare Delaunay triangulation libraries
 *
 * Copyright (c) 2023 Pierre DEJOUE
 ******************************************************************************/

#include "draw_shape.h"
#include "renderer.h"
#include "settings.h"
#include "shape_control_window.h"
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

#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>


#ifdef _WIN32
#undef min
#undef max
#endif

namespace
{

const ImColor WindowBackgroundColor_Classic(29, 75, 99, 255);
const ImColor WindowBackgroundColor_Dark(4, 8, 25, 255);
const ImColor WindowMainBackgroundColor_Classic(35, 92, 121, 255);
const ImColor WindowMainBackgroundColor_Dark(10, 25, 50, 255);

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

    // Viewport window
    ViewportWindow viewport_window;

    using scalar = ViewportWindow::scalar;

    // Register the Delaunay triangulation implementations
    {
        const bool registered_delaunay_impl = delaunay::register_all_implementations();
        assert(registered_delaunay_impl);
        if (!registered_delaunay_impl)
            std::cerr << "Error during Delaunay implementations' registration" << std::endl;
    }

    // Renderer
    renderer::Draw2D draw_2d_renderer(&err_handler);
    if (!draw_2d_renderer.init())
        return 1;
    draw_2d_renderer.set_background_color(0.166f, 0.166f, 0.166f);

    // Main loop
    std::unique_ptr<ShapeWindow> shape_control_window;
    ScreenPos initial_window_pos(0.f, 0.f);
    ViewportWindow::Key previously_selected_tab;
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
                shapes::io::ShapeAggregate<double> shapes;
                std::string filename = "no_file";
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
                            filename = std::filesystem::path(path).filename().string();
                            shapes.clear();
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
                        filename = std::filesystem::path(path).filename().string();
                        shapes = shapes::io::dat::parse_shapes_from_file(path, io_err_handler);
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
                        filename = std::filesystem::path(path).filename().string();
                        shapes.clear();
                        shapes.reserve(file_paths.point_paths.size() + file_paths.cubic_bezier_paths.size());
                        for (const auto& pp : file_paths.point_paths)
                            shapes.emplace_back(std::move(pp));
                        for (const auto& cbp : file_paths.cubic_bezier_paths)
                            shapes.emplace_back(std::move(cbp));

                    }
                }
                if (!shapes.empty())
                {
                    shape_control_window = std::make_unique<ShapeWindow>(filename, initial_window_pos, std::move(shapes), viewport_window);
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
            initial_window_pos.y = ImGui::GetWindowHeight() + 1.f;
            ImGui::EndMainMenuBar();
        }

        // Settings window (always ON)
        {
            bool can_be_erased = false;
            settings.visit_window(can_be_erased, initial_window_pos);
        }
        viewport_window.set_initial_window_pos_size(initial_window_pos, ScreenPos());

        // Shape control window
        bool geometry_has_changed = false;
        if(shape_control_window)
        {
            bool can_be_erased = false;
            shape_control_window->visit(can_be_erased, settings, geometry_has_changed);
            if (can_be_erased)
            {
                shape_control_window.reset();           // Close window
                viewport_window.reset();                // Not closed, just reset
                previously_selected_tab = "";
            }
        }

        // Fwd draw commands to Viewport
        const bool imgui_rendering = settings.get_general_settings()->imgui_renderer;
        if(shape_control_window)
        {
            for (const auto& [key, cmds] : shape_control_window->get_draw_command_lists())
            {
                DrawCommands<scalar> filtered_cmds;
                std::copy_if(std::cbegin(cmds), std::cend(cmds), std::back_inserter(filtered_cmds), [imgui_rendering](const auto& cmd) { return imgui_rendering || is_bezier_path(*cmd.shape); });
                viewport_window.set_draw_commands(key, std::move(filtered_cmds));
            }
        }

        // Viewport window
        bool can_be_erased = false;
        ViewportWindow::Key selected_tab;
        viewport_window.visit(can_be_erased, settings, selected_tab);

        if (shape_control_window && !imgui_rendering)
        {
            const auto& dcls = shape_control_window->get_draw_command_lists();
            const auto draw_commands_it = std::find_if(std::cbegin(dcls), std::cend(dcls), [&selected_tab](const auto& kvp) { return kvp.first == selected_tab; });
            assert(draw_commands_it != std::cend(dcls));
            update_opengl_draw_list<scalar>(draw_2d_renderer.draw_list(), draw_commands_it->second, (geometry_has_changed || (selected_tab != previously_selected_tab)), settings);
            previously_selected_tab = selected_tab;
        }

#if DELAUNAY_VIEWER_IMGUI_DEMO_FLAG
        // Dear Imgui Demo
        ImGui::ShowDemoWindow();
#endif

        // Prepare ImGui rendering
        ImGui::Render();

        // OpenGL frame setup
        int display_w, display_h;
        glfwGetFramebufferSize(glfw_context.window(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        const auto clear_color = static_cast<ImVec4>(imgui_dark_mode ? WindowMainBackgroundColor_Dark : WindowMainBackgroundColor_Classic);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // Viewport rendering
        if (display_w > 0 && display_h > 0)
        {
            // Rendering flags
            renderer::Flag::type flags = 0;
            if (settings.get_general_settings()->flip_y) { flags |= renderer::Flag::FlipYAxis; }
            if (!shape_control_window || settings.get_general_settings()->imgui_renderer) { flags |= renderer::Flag::OnlyBackground; }

            // Render
            const auto target_bb = shapes::cast<float, scalar>(viewport_window.get_canvas_bounding_box());
            const auto screen_bb = viewport_window.get_viewport_bounding_box();
            const auto canvas = Canvas(screen_bb.min(), screen_bb.extent(), target_bb);
            draw_2d_renderer.render(canvas, static_cast<float>(display_h), flags);
        }

        // Imgui rendering (always on top of the viewport rendering)
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

