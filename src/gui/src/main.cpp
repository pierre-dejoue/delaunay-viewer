/*******************************************************************************
 * DELAUNAY VIEWER
 *
 * A tool to test and compare Delaunay triangulation libraries
 *
 * Copyright (c) 2023 Pierre DEJOUE
 ******************************************************************************/

#include "argagg_wrap.h"
#include "draw_shape.h"
#include "project.h"
#include "renderer.h"
#include "settings.h"
#include "settings_window.h"
#include "shape_control_window.h"
#include "style.h"
#include "window_layout.h"

#include <dt/dt_impl.h>
#include <shapes/bounding_box.h>
#include <shapes/bounding_box_algos.h>
#include <shapes/io.h>
#include <stdutils/algorithm.h>
#include <stdutils/io.h>
#include <stdutils/macros.h>
#include <stdutils/platform.h>
#include <svg/svg.h>

// Order matters in this section
#include <imgui_wrap.h>
#include "imgui_helpers.h"
#include <pfd_wrap.h>
#include "opengl_and_glfw.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace
{

void err_callback(stdutils::io::SeverityCode sev, std::string_view msg)
{
    std::cerr << stdutils::io::str_severity_code(sev) << ": " << msg << std::endl;
}

std::string project_title()
{
    std::stringstream title;
    title << project::get_short_desc() << ' ' << project::get_version_string();
    return title.str();
}

argagg::parser argparser{ {
    { "help", { "-h", "--help" }, "Print usage note and exit", 0 },
    { "version", { "--version" }, "Print version and exit", 0 },
    { "platform", { "--platform" }, "Print platform information and exit", 0 }
} };

void usage_notes(std::ostream& out)
{
    out << project_title() << "\n\n";
    out << "Options:\n\n";
    out << argparser;
}

argagg::parser_results parse_command_line(int argc, char *argv[])
{
    argagg::parser_results args;
    try
    {
        args = argparser.parse(argc, argv);
    }
    catch (const std::exception& e)
    {
        usage_notes(std::cerr);
        std::stringstream out;
        out << "While parsing arguments: " << e.what();
        err_callback(stdutils::io::Severity::EXCPT, out.str());
        std::exit(EXIT_FAILURE);
    }
    return args;
}

// Application windows
struct AppWindows
{
    std::unique_ptr<SettingsWindow> settings;
    std::unique_ptr<ViewportWindow> viewport;
    std::unique_ptr<ShapeWindow> shape_control;
    struct
    {
        WindowLayout settings;
        WindowLayout viewport;
        WindowLayout shape_control;
    } layout;
};

void main_menu_bar(AppWindows& windows, renderer::Draw2D& renderer, bool& application_should_close, bool& gui_dark_mode)
{
    application_should_close = false;
    if (ImGui::BeginMainMenuBar())
    {
        const stdutils::io::ErrorHandler io_err_handler(err_callback);
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
            stdutils::erase_if(shapes, [](const auto& shape) {
                if (shapes::get_dimension(shape) != 2)
                {
                    std::stringstream out;
                    out << "Input shape of type " << shapes::get_type_str(shape) << " is not supported and was filtered out";
                    err_callback(stdutils::io::Severity::ERR, out.str());
                    return true;
                }
                return false;
            });
            if (!shapes.empty() && windows.viewport)
            {
                windows.viewport->reset();
                renderer.draw_list().reset();
                windows.shape_control = std::make_unique<ShapeWindow>(filename, std::move(shapes), *windows.viewport);
            }
            ImGui::Separator();
            bool save_menu_enabled = static_cast<bool>(windows.shape_control);
            if (ImGui::MenuItem("Save as DAT", "", false, save_menu_enabled))
            {
                std::filesystem::path filepath = pfd::save_file(
                    "Select a file", "",
                    { "DAT", "*.dat" },
                    pfd::opt::force_overwrite).result();
                if (!filepath.empty())
                {
                    if (!filepath.has_extension()) { filepath.replace_extension("dat"); }
                    shapes::io::dat::save_shapes_as_file(filepath, windows.shape_control->get_triangulation_input_aggregate(), io_err_handler);
                }
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Options"))
            {
                if (ImGui::MenuItem("Dark Mode", "", &gui_dark_mode))
                {
                    imgui_set_style(gui_dark_mode);
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            application_should_close = ImGui::MenuItem("Quit", "Alt+F4");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

} // Anonymous namespace

int main(int argc, char *argv[])
{
    // Parse command line
    const auto args = parse_command_line(argc, argv);
    if (args["help"])
    {
        usage_notes(std::cout);
        return EXIT_SUCCESS;
    }
    if (args["version"])
    {
        std::cout << project_title() << std::endl;
        stdutils::platform::print_compiler_all_info(std::cout);
        return EXIT_SUCCESS;
    }
    if (args["platform"])
    {
        stdutils::platform::print_platform_info(std::cout);
        return EXIT_SUCCESS;
    }

    // Create GLFW window and load OpenGL
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    stdutils::io::ErrorHandler err_handler(err_callback);
    bool any_fatal_err = false;
    GLFWWindowContext glfw_context = create_glfw_window_load_opengl(WINDOW_WIDTH, WINDOW_HEIGHT, project_title(), any_fatal_err, &err_handler);
    if (any_fatal_err)
        return EXIT_FAILURE;
    assert(glfw_context.window() != nullptr);

    // Setup Dear ImGui context
    const DearImGuiContext dear_imgui_context(glfw_context.window(), any_fatal_err);
    if (any_fatal_err)
        return EXIT_FAILURE;

    // Print out project version and backend information
    std::cout << project_title() << std::endl;
    dear_imgui_context.backend_info(std::cout);

    // GUI Style
    bool gui_dark_mode = false;
    imgui_set_style(gui_dark_mode);

    // Application Settings
    Settings settings;

    // Application Windows
    AppWindows windows;
    windows.settings = std::make_unique<SettingsWindow>(settings);
    windows.viewport = std::make_unique<ViewportWindow>();
    constexpr float WINDOW_SETTINGS_WIDTH = 400.f;
    constexpr float WINDOW_SETTINGS_HEIGHT = 400.f;
    windows.layout.settings      = WindowLayout(0.f,                   0.f,                    WINDOW_SETTINGS_WIDTH, WINDOW_SETTINGS_HEIGHT);
    windows.layout.viewport      = WindowLayout(WINDOW_SETTINGS_WIDTH, 0.f,                    -1.f,                  -1.f);
    windows.layout.shape_control = WindowLayout(0.f,                   WINDOW_SETTINGS_HEIGHT, WINDOW_SETTINGS_WIDTH, -1.f);

    // Steiner callback
    using scalar = ViewportWindow::scalar;
    windows.viewport->set_steiner_callback([&windows, &err_handler](const shapes::Point2d<scalar>& p) {
        if (windows.shape_control) { windows.shape_control->add_steiner_point(p); }
        else { err_handler(stdutils::io::Severity::WARN, "Could not add steiner point: No control window"); }
    });

    // Register the Delaunay triangulation implementations
    if (!delaunay::register_all_implementations())
    {
        err_handler(stdutils::io::Severity::FATAL, "Issue during Delaunay implementations' registration");
        return EXIT_FAILURE;
    }

    // Renderer
    renderer::Draw2D draw_2d_renderer(&err_handler);
    if (!draw_2d_renderer.init())
    {
        err_handler(stdutils::io::Severity::FATAL, "Failed to initialize the renderer");
        return EXIT_FAILURE;
    }

    // Main loop
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
        dear_imgui_context.new_frame();

        // Main menu
        {
            bool app_should_close = false;
            main_menu_bar(windows, draw_2d_renderer, app_should_close, gui_dark_mode);
            if (app_should_close)
                glfwSetWindowShouldClose(glfw_context.window(), 1);
        }

        // Settings window
        if (windows.settings)
        {
            bool can_be_erased = false;
            windows.settings->visit(can_be_erased, windows.layout.settings);
            assert(!can_be_erased); // Always ON
        }

        // Shape control window
        bool geometry_has_changed = false;
        if (windows.shape_control)
        {
            assert(windows.viewport);
            bool can_be_erased = false;
            windows.shape_control->visit(can_be_erased, settings, windows.layout.shape_control, geometry_has_changed);
            if (can_be_erased)
            {
                windows.shape_control.reset();              // Close window
                windows.viewport->reset();                  // Not closed, just reset
                draw_2d_renderer.draw_list().reset();
                previously_selected_tab = "";
            }
        }

        // Forward a selection of draw commands to the Viewport. Those will be rendered by ImGui on top of our rendering.
        if (windows.shape_control)
        {
            assert(windows.viewport);
            const bool imgui_rendering = settings.get_general_settings()->imgui_renderer;
            for (const auto& [key, cmds] : windows.shape_control->get_draw_command_lists())
            {
                DrawCommands<scalar> filtered_cmds;
                std::copy_if(std::cbegin(cmds), std::cend(cmds), std::back_inserter(filtered_cmds), [imgui_rendering](const auto& cmd) { return imgui_rendering || is_bezier_path(*cmd.shape); });
                windows.viewport->set_draw_commands(key, std::move(filtered_cmds));
            }
        }

        // Viewport window
        ViewportWindow::Key selected_tab;
        if (windows.viewport)
        {
            bool can_be_erased = false;
            windows.viewport->visit(can_be_erased, settings, windows.layout.viewport, selected_tab);
            assert(!can_be_erased);     // Always ON
            draw_2d_renderer.set_background_color(windows.viewport->get_background_color());
        }

        // Transfer draw lists to our renderer
        if (windows.shape_control)
        {
            const auto& dcls = windows.shape_control->get_draw_command_lists();
            const auto draw_commands_it = std::find_if(std::cbegin(dcls), std::cend(dcls), [&selected_tab](const auto& kvp) { return kvp.first == selected_tab; });
            if (draw_commands_it != std::cend(dcls))
            {
                const bool update_buffers = geometry_has_changed || (selected_tab != previously_selected_tab);
                update_opengl_draw_list<scalar>(draw_2d_renderer.draw_list(), draw_commands_it->second, update_buffers, settings);
                previously_selected_tab = selected_tab;
            }
        }

#if DELAUNAY_VIEWER_IMGUI_DEMO_FLAG
        // Dear ImGui Demo
        ImGui::ShowDemoWindow();
#endif

        // OpenGL frame setup
        int display_w, display_h;
        glfwGetFramebufferSize(glfw_context.window(), &display_w, &display_h);
        const bool is_minimized = (display_w <= 0 || display_h <= 0);
        glViewport(0, 0, display_w, display_h);
        const auto clear_color = get_background_color(gui_dark_mode);
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        // Viewport rendering
        if (windows.viewport && !is_minimized)
        {
            // Rendering flags
            renderer::Flag::type flags = 0;
            if (settings.get_general_settings()->flip_y) { flags |= renderer::Flag::FlipYAxis; }
            if (!windows.shape_control || settings.get_general_settings()->imgui_renderer) { flags |= renderer::Flag::OnlyBackground; }

            // Render
            const auto target_bb = shapes::cast<float, scalar>(windows.viewport->get_canvas_bounding_box());
            const auto screen_bb = windows.viewport->get_viewport_bounding_box();
            const auto canvas = Canvas(screen_bb.min(), screen_bb.extent(), target_bb);
            draw_2d_renderer.render(canvas, static_cast<float>(display_h), flags);
        }

        // ImGui rendering (always on top of the viewport rendering)
        dear_imgui_context.render();

        // End frame
        glfwSwapBuffers(glfw_context.window());
    } // while (!glfwWindowShouldClose(glfw_context.window()))

    return EXIT_SUCCESS;
}

