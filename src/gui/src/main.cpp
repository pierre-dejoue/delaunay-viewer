/*******************************************************************************
 * DELAUNAY VIEWER
 *
 * A tool to test and compare Delaunay triangulation libraries
 *
 * Copyright (c) 2023 Pierre DEJOUE
 * This code is distributed under the terms of the MIT License
 ******************************************************************************/

#include "argagg_wrap.h"
#include "drawing_settings.h"
#include "dt_tracker.h"
#include "project.h"
#include "renderer.h"
#include "renderer_helpers.h"
#include "settings.h"
#include "settings_window.h"
#include "shape_control_window.h"
#include "style.h"
#include "viewport_window.h"
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
#include <string_view>
#include <vector>

namespace {

void err_callback(stdutils::io::SeverityCode sev, std::string_view msg)
{
    std::cerr << stdutils::io::str_severity_code(sev) << ": " << msg << std::endl;
}

namespace details {

    std::string s_project_title()
    {
        std::stringstream title;
        title << project::get_short_desc() << ' ' << project::get_version_string();
        return title.str();
    }

} // namespace details

std::string_view project_title()
{
    static std::string project_title = details::s_project_title();
    return project_title;
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

using scalar = ViewportWindow::scalar;

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

void main_menu_bar(AppWindows& windows, renderer::Draw2D& renderer, const DtTracker<scalar>& dt_tracker, bool& application_should_close, bool& gui_dark_mode)
{
    std::string filename = "no_file";
    application_should_close = false;
    using scalar = ViewportWindow::scalar;
    shapes::io::ShapeAggregate<scalar> shapes;
    if (ImGui::BeginMainMenuBar())
    {
        const stdutils::io::ErrorHandler io_err_handler(err_callback);
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open CDT"))
            {
                const auto paths = pfd::source_paths(
                    pfd::open_file("Select a CDT file", "", { "CDT file", "*.cdt", "All files", "*.*" })
                );
                for (const auto& path : paths)
                {
                    std::cout << "User selected CDT file " << path << std::endl;
                    if (shapes::io::cdt::peek_point_dimension(path, io_err_handler) == 2)
                    {
                        auto cdt_shapes = shapes::io::cdt::parse_2d_shapes_from_file(path, io_err_handler);
                        filename = path.filename().string();
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
                const auto paths = pfd::source_paths(
                    pfd::open_file("Select a DAT file", "", { "DAT file", "*.dat", "All files", "*.*" })
                );
                for (const auto& path : paths)
                {
                    std::cout << "User selected DAT file " << path << std::endl;
                    filename = path.filename().string();
                    shapes = shapes::io::dat::parse_shapes_from_file(path, io_err_handler);
                }
            }
            if (ImGui::MenuItem("Open SVG"))
            {
                const auto paths = pfd::source_paths(
                    pfd::open_file("Select a SVG file", "", { "SVG file", "*.svg" })
                );
                for (const auto& path : paths)
                {
                    std::cout << "User selected SVG file " << path << std::endl;
                    auto file_paths = svg::io::parse_svg_paths(path, io_err_handler);
                    std::cout << "Nb of point paths: " << file_paths.point_paths.size() << ". Nb of cubic bezier paths: " << file_paths.cubic_bezier_paths.size() << "." << std::endl;
                    filename = path.filename().string();
                    shapes.clear();
                    shapes.reserve(file_paths.point_paths.size() + file_paths.cubic_bezier_paths.size());
                    for (const auto& pp : file_paths.point_paths)
                        shapes.emplace_back(std::move(pp));
                    for (const auto& cbp : file_paths.cubic_bezier_paths)
                        shapes.emplace_back(std::move(cbp));
                }
            }
            stdutils::erase_if(shapes, [](const auto& shape_wrapper) {
                const auto& shape = shape_wrapper.shape;
                if (shapes::get_dimension(shape) != 2)
                {
                    std::stringstream out;
                    out << "Input shape of type " << shapes::get_type_str(shape) << " is not supported and was filtered out";
                    err_callback(stdutils::io::Severity::ERR, out.str());
                    return true;
                }
                return false;
            });
            ImGui::Separator();
            bool save_input_as_dat_menu_enabled = static_cast<bool>(windows.shape_control);
            if (ImGui::MenuItem("Save input as DAT", "", false, save_input_as_dat_menu_enabled))
            {
                std::filesystem::path path = pfd::target_path(
                    pfd::save_file("Select a file", "", { "DAT", "*.dat" }, pfd::opt::force_overwrite)
                );
                if (!path.empty())
                {
                    if (!path.has_extension()) { path.replace_extension("dat"); }
                    shapes::io::dat::save_shapes_as_file(path, windows.shape_control->get_triangulation_input_aggregate(), io_err_handler);
                }
            }
            bool save_tab_as_dat_menu_enabled = static_cast<bool>(windows.viewport) && static_cast<bool>(windows.shape_control);
            if (ImGui::MenuItem("Save current viewport as DAT", "", false, save_tab_as_dat_menu_enabled))
            {
                std::filesystem::path path = pfd::target_path(
                    pfd::save_file("Select a file", "", { "DAT", "*.dat" }, pfd::opt::force_overwrite)
                );
                if (!path.empty())
                {
                    if (!path.has_extension()) { path.replace_extension("dat"); }
                    const auto key = windows.viewport->get_latest_selected_tab();
                    shapes::io::dat::save_shapes_as_file(path, windows.shape_control->get_tab_aggregate(key), io_err_handler);
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
    if (!shapes.empty() && windows.viewport)
    {
        windows.viewport->reset();
        renderer.draw_list().reset();
        windows.shape_control = std::make_unique<ShapeWindow>(filename, std::move(shapes), dt_tracker, *windows.viewport);
    }
}

} // namespace

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
    stdutils::io::ErrorHandler err_handler(err_callback);
    bool any_fatal_err = false;
    unsigned int back_framebuffer_id = 0;
    GLFWWindowContext glfw_context = [&back_framebuffer_id, &any_fatal_err, &err_handler]() {
        constexpr int WINDOW_WIDTH = 1280;
        constexpr int WINDOW_HEIGHT = 720;
        GLFWOptions options;
        options.title = project_title();
        auto context = create_glfw_window_load_opengl(WINDOW_WIDTH, WINDOW_HEIGHT, options, any_fatal_err, back_framebuffer_id, &err_handler);
        return context;
    }();
    if (any_fatal_err || glfw_context.window() == nullptr)
        return EXIT_FAILURE;

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

    // Register the Delaunay triangulation implementations
    if (!delaunay::register_all_implementations())
    {
        err_handler(stdutils::io::Severity::FATAL, "Issue during Delaunay implementations' registration");
        return EXIT_FAILURE;
    }
    DtTracker<scalar> dt_tracker;

    // Application Settings
    Settings settings;

    // Application Windows
    AppWindows windows;
    windows.settings = std::make_unique<SettingsWindow>(settings, dt_tracker);
    windows.viewport = std::make_unique<ViewportWindow>();
    constexpr float WINDOW_SETTINGS_WIDTH = 400.f;
    constexpr float WINDOW_SETTINGS_HEIGHT = 450.f;
    windows.layout.settings      = WindowLayout(0.f,                   0.f,                    WINDOW_SETTINGS_WIDTH, WINDOW_SETTINGS_HEIGHT);
    windows.layout.viewport      = WindowLayout(WINDOW_SETTINGS_WIDTH, 0.f,                    -1.f,                  -1.f);
    windows.layout.shape_control = WindowLayout(0.f,                   WINDOW_SETTINGS_HEIGHT, WINDOW_SETTINGS_WIDTH, -1.f);

    // Steiner callback
    windows.viewport->set_steiner_callback([&windows, &err_handler](const shapes::Point2d<scalar>& p) {
        if (windows.shape_control) { windows.shape_control->add_steiner_point(p); }
        else { err_handler(stdutils::io::Severity::WARN, "Could not add steiner point: No control window"); }
    });

    // Renderer
    renderer::Draw2D draw_2d_renderer(&err_handler);
    if (!draw_2d_renderer.init(back_framebuffer_id))
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
            main_menu_bar(windows, draw_2d_renderer, dt_tracker, app_should_close, gui_dark_mode);
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
        if (windows.viewport)
        {
            bool can_be_erased = false;
            windows.viewport->visit(can_be_erased, settings, windows.layout.viewport);
            assert(!can_be_erased);     // Always ON
            draw_2d_renderer.set_viewport_background_color(windows.viewport->get_background_color());
        }

        // Transfer draw lists to our renderer
        const auto drawing_options = drawing_options_from_settings(settings);
        if (windows.shape_control)
        {
            const auto& dcls = windows.shape_control->get_draw_command_lists();
            ViewportWindow::Key selected_tab = windows.viewport ? windows.viewport->get_latest_selected_tab() : ViewportWindow::Key();
            const auto draw_commands_it = std::find_if(std::cbegin(dcls), std::cend(dcls), [&selected_tab](const auto& kvp) { return kvp.first == selected_tab; });
            if (draw_commands_it != std::cend(dcls))
            {
                const bool update_buffers = geometry_has_changed || (selected_tab != previously_selected_tab);
                update_opengl_draw_list<scalar>(draw_2d_renderer.draw_list(), draw_commands_it->second, update_buffers, drawing_options);
                previously_selected_tab = selected_tab;
            }
        }

#if DELAUNAY_VIEWER_IMGUI_DEMO_FLAG
        // Dear ImGui Demo
        ImGui::ShowDemoWindow();
#endif

        // OpenGL frame setup
        const auto [display_w, display_h] = glfw_context.window_size();
        const bool is_minimized = (display_w <= 0 || display_h <= 0);
        if(!draw_2d_renderer.init_framebuffer(display_w, display_h))
        {
            err_handler(stdutils::io::Severity::FATAL, "Failed to initialize the framebuffer");
            return EXIT_FAILURE;
        }
        draw_2d_renderer.clear_framebuffer(get_window_background_color(gui_dark_mode));

        // Viewport rendering
        if (windows.viewport && !is_minimized)
        {
            // Rendering flags
            renderer::Flag::type flags = renderer::Flag::ViewportBackground;;
            if (settings.get_general_settings()->flip_y) { flags |= renderer::Flag::FlipYAxis; }
            const bool only_background = !windows.shape_control || settings.get_general_settings()->imgui_renderer;

            // Render
            const auto target_bb = shapes::cast<float, scalar>(windows.viewport->get_canvas_bounding_box());
            const auto screen_bb = windows.viewport->get_viewport_bounding_box();
            const auto viewport_canvas = Canvas(screen_bb.min(), screen_bb.extent(), target_bb);
            stable_sort_draw_commands(draw_2d_renderer.draw_list());
            if (only_background)
            {
                draw_2d_renderer.render_viewport_background(viewport_canvas);
            }
            else
            {
                draw_2d_renderer.render(viewport_canvas, flags);
            }
        }

        // ImGui rendering (always on top of the viewport rendering)
        dear_imgui_context.render();

        // End frame
        glfwSwapBuffers(glfw_context.window());
    } // while (!glfwWindowShouldClose(glfw_context.window()))

    return EXIT_SUCCESS;
}

