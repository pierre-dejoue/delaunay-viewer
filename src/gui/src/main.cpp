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
#include <stdutils/macros.h>

#include <portable-file-dialogs.h>      // Include before glfw3.h
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>

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

void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

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

} // Anonymous namespace


int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    // Versions
    std::stringstream delaunay_title;
    delaunay_title << "Delaunay Viewer " << get_version_string();
    std::cout << delaunay_title.str() << std::endl;
    std::cout << "Dear ImGui " << IMGUI_VERSION << std::endl;

    // Setup main window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, delaunay_title.str().c_str(), NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;   // Unused

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Style
    bool imgui_dark_mode = false;
    imgui_set_style(imgui_dark_mode);

    // Open settings window
    Settings settings;
    settings.open_window();

    {
        // TODO move to unit tests
        shapes::Range<double> r1; r1.add(1.0);
        shapes::ensure_min_extent(r1);
        std::cout << r1 << std::endl;

        shapes::Range<double> r2; r2.add(123456.0);
        shapes::ensure_min_extent(r2);
        std::cout << r2 << std::endl;

        shapes::Range<double> r3; r3.add(-73321.0);
        shapes::ensure_min_extent(r3);
        std::cout << r3 << std::endl;
    }

    {
        // Register the Delaunay triangulation implementations
        const bool registered_delaunay_impl = delaunay::register_all_implementations();
        assert(registered_delaunay_impl);
    }

    // Main loop
    std::vector<std::unique_ptr<ShapeWindow>> shape_windows;
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main menu
        if (ImGui::BeginMainMenuBar())
        {
            stdutils::io::ErrorHandler io_err_handler = [](stdutils::io::SeverityCode, std::string_view msg) { std::cerr << msg << std::endl; };
            if (ImGui::BeginMenu("File"))
            {
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
                if (ImGui::MenuItem("Open DAT"))
                {
                    const auto paths = pfd::open_file("Select a DAT file", "",
                        { "DAT file", "*.dat", "All files", "*.*" }).result();
                    for (const auto& path : paths)
                    {
                        std::cout << "User selected DAT file " << path << std::endl;
                        auto shapes = shapes::io::parse_shapes_dat_file(path, io_err_handler);
                        const std::string filename = std::filesystem::path(path).filename().string();
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
                    glfwSetWindowShouldClose(window, 1);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Shape windows (one window per input file)
        for (auto it = std::begin(shape_windows); it != std::end(shape_windows);)
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
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        const auto clear_color = static_cast<ImVec4>(imgui_dark_mode ? WindowMainBackgroundColor_Dark : WindowMainBackgroundColor_Classic);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    } // while (!glfwWindowShouldClose(window))

    // Cleanup
    shape_windows.clear();
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

