/*******************************************************************************
 * DELAUNAY VIEWER
 *
 * A tool to test and compare Delaunay triangulation libraries
 *
 * Copyright (c) 2023 Pierre DEJOUE
 ******************************************************************************/

#include "settings.h"
#include "shape_control_window.h"
#include "version.h"

#include <dt/dt_impl.h>
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
    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

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

    // Main loop
    std::vector<std::unique_ptr<ShapeWindow>> shape_windows;
    while (!glfwWindowShouldClose(glfw_context.window()))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

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
                            shape_windows.emplace_back(std::make_unique<ShapeWindow>(std::move(shapes), filename));
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
                        shape_windows.emplace_back(std::make_unique<ShapeWindow>(std::move(shapes), filename));
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
                        shape_windows.emplace_back(std::make_unique<ShapeWindow>(std::move(shapes), filename));
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

        // Shape windows (one window per input file)
        for (auto it = std::cbegin(shape_windows); it != std::cend(shape_windows);)
        {
            bool can_be_erased = false;
            (*it)->visit(can_be_erased, settings);
            it = can_be_erased ? shape_windows.erase(it) : std::next(it);
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

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(glfw_context.window(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        const auto clear_color = static_cast<ImVec4>(imgui_dark_mode ? WindowMainBackgroundColor_Dark : WindowMainBackgroundColor_Classic);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(glfw_context.window());
    } // while (!glfwWindowShouldClose(window))

    // Cleanup
    shape_windows.clear();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

