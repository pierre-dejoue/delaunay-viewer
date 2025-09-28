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
#include "gui_style.h"
#include "project.h"
#include "renderer.h"
#include "renderer_helpers.h"
#include "settings.h"
#include "settings_window.h"
#include "shape_control_window.h"
#include "viewport_window.h"

#include <dt/dt_impl.h>
#include <gui/abstract/canvas.h>
#include <gui/abstract/window_layout.h>
#include <gui/base/opengl_and_glfw.h>
#include <gui/base/pfd_wrap.h>
#include <imgui/imgui.h>
#include <imgui/key_shortcut.h>
#include <shapes/bounding_box.h>
#include <shapes/bounding_box_algos.h>
#include <shapes/io.h>
#include <stdutils/algorithm.h>
#include <stdutils/io.h>
#include <stdutils/macros.h>
#include <stdutils/platform.h>
#include <stdutils/time.h>
#include <svg/svg.h>

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
        title << project::get_name() << ' ' << project::get_version_string();
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
            application_should_close = ImGui::MenuItem("Quit", ImGui::key_shortcut::quit().label);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if (!shapes.empty() && windows.viewport)
    {
        windows.viewport->reset();
        renderer.draw_list().clear_all();
        windows.shape_control = std::make_unique<ShapeWindow>(filename, std::move(shapes), dt_tracker);
        windows.viewport->set_geometry_bounding_box(windows.shape_control->get_geometry_bounding_box());
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
        options.default_title = project_title();
        options.maximize_window = true;
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
    std::cout << "Current time: " << stdutils::current_local_date_and_time() << std::endl;
    dear_imgui_context.backend_info(std::cout);
    std::cout.flush();

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

// On macOS, use the touchpad to pan and zoom the viewport image
// On Windows and Linux, use the mouse wheel to zoom the viewport image
#if defined(__APPLE__)
    // Scroll event callback
    glfw_context.set_scroll_event_callback([&windows](ScreenVect dir) {
        windows.viewport->signal_scroll_event(dir);
    });
    if constexpr (GLFWWindowContext::supports_trackpad_zoom_events())
    {
        // Zoom event callback
        glfw_context.set_zoom_event_callback([&windows](float scale) {
            windows.viewport->signal_zoom_event(static_cast<scalar>(scale));
        });
    }
#else
    // Scroll event callback
    glfw_context.set_scroll_event_callback([&windows](ScreenVect dir) {
        const auto zoom_scale = (dir.y > 0 ? 1.1 : (dir.y < 0 ? 0.9 : 1.0));
        windows.viewport->signal_zoom_event(static_cast<scalar>(zoom_scale));
    });
#endif


    // Renderer
    renderer::Draw2D::Settings renderer_settings;
    renderer_settings.back_framebuffer_id = back_framebuffer_id;
    renderer_settings.line_smooth = settings.get_general_settings()->line_smooth;
    auto draw_2d_renderer = std::make_unique<renderer::Draw2D>(renderer_settings, &err_handler);
    if (!draw_2d_renderer || !draw_2d_renderer->initialized())
    {
        err_handler(stdutils::io::Severity::FATAL, "Failed to initialize the renderer");
        return EXIT_FAILURE;
    }
    CBPSegmentation<scalar> cbp_segmentation;

    // Main loop
    ViewportWindow::Key previously_selected_tab;
    ViewportWindow::TabList tab_list;
    float framebuffer_scale{1};
    GLFWvidmode glfw_vid_mode{};
    while (!glfwWindowShouldClose(glfw_context.window()))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Window, display, monitor status
        const auto window_status = glfw_context.window_status();
        const auto [display_w, display_h] = glfw_context.framebuffer_size();
        if (window_status.is_minimized || display_w == 0 || display_h == 0)
        {
            dear_imgui_context.sleep(10);
            continue;
        }
        const GLFWvidmode* glfw_vid_mode_ptr = glfwGetVideoMode(glfwGetPrimaryMonitor());
        assert(glfw_vid_mode_ptr);
        if (glfw_vid_mode_ptr && (glfw_vid_mode != *glfw_vid_mode_ptr))
        {
            glfw_vid_mode = *glfw_vid_mode_ptr;
            std::stringstream out;
            out << "Monitor: " << glfw_vid_mode;
            err_handler(stdutils::io::Severity::TRACE, out.str());
        }
        const bool framebuffer_scale_changed = glfw_context.get_framebuffer_scale(framebuffer_scale);
        if (framebuffer_scale_changed)
        {
            std::stringstream out;
            out << "Content scale: " << framebuffer_scale << "x";
            err_handler(stdutils::io::Severity::TRACE, out.str());
        }


        // Start the Dear ImGui frame
        dear_imgui_context.new_frame();

        // Main menu
        {
            bool app_should_close = false;
            main_menu_bar(windows, *draw_2d_renderer, dt_tracker, app_should_close, gui_dark_mode);
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

        // Renderer update
        const bool line_smooth_settings = settings.get_general_settings()->line_smooth;
        if (renderer_settings.line_smooth != line_smooth_settings)
        {
            renderer_settings.line_smooth = line_smooth_settings;
            draw_2d_renderer = std::make_unique<renderer::Draw2D>(renderer_settings, &err_handler);
            if (!draw_2d_renderer || !draw_2d_renderer->initialized())
            {
                err_handler(stdutils::io::Severity::FATAL, "Failed to initialize the renderer");
                return EXIT_FAILURE;
            }
        }
        assert(draw_2d_renderer);

        // Shape control window
        bool geometry_has_changed = false;
        tab_list.clear();
        if (windows.shape_control)
        {
            assert(windows.viewport);
            bool can_be_erased = false;
            windows.shape_control->visit(can_be_erased, settings, windows.layout.shape_control, geometry_has_changed);
            if (can_be_erased)
            {
                windows.shape_control.reset();              // Close window
                windows.viewport->reset();                  // Not closed, just reset
                draw_2d_renderer->draw_list().clear_all();
                cbp_segmentation.clear_all();
                previously_selected_tab = "";
            }
            else
            {
                const auto& dcls = windows.shape_control->get_draw_command_lists();
                std::transform(std::cbegin(dcls), std::cend(dcls), std::back_inserter(tab_list), [](const auto& kvp) { return kvp.first; });
            }
        }

        // Viewport window
        if (windows.viewport)
        {
            bool can_be_erased = false;
            windows.viewport->visit(can_be_erased, tab_list, settings, windows.layout.viewport);
            assert(!can_be_erased);     // Always ON
            draw_2d_renderer->set_viewport_background_color(windows.viewport->get_background_color());
        }

        // Transfer draw lists to our renderer
        const auto drawing_options = drawing_options_from_settings(settings);
        const DrawCommands<scalar>* draw_commands_ptr = nullptr;
        if (windows.shape_control)
        {
            const auto& dcls = windows.shape_control->get_draw_command_lists();
            ViewportWindow::Key selected_tab = windows.viewport ? windows.viewport->get_latest_selected_tab() : ViewportWindow::Key();
            const auto draw_commands_it = std::find_if(std::cbegin(dcls), std::cend(dcls), [&selected_tab](const auto& kvp) { return kvp.first == selected_tab; });
            if (draw_commands_it != std::cend(dcls))
            {
                if (selected_tab != previously_selected_tab) { geometry_has_changed = true; }
                draw_commands_ptr = &draw_commands_it->second;
                previously_selected_tab = selected_tab;
            }
        }

#if DELAUNAY_VIEWER_IMGUI_DEMO_FLAG
        // Dear ImGui Demo
        ImGui::ShowDemoWindow();
#endif

        // OpenGL frame setup
        if(!draw_2d_renderer->init_framebuffer(display_w, display_h))
        {
            err_handler(stdutils::io::Severity::FATAL, "Failed to initialize the framebuffer");
            return EXIT_FAILURE;
        }
        draw_2d_renderer->clear_framebuffer(gui_style::get_window_background_color(gui_dark_mode));

        // Viewport rendering
        if (windows.viewport)
        {
            // Rendering flags
            renderer::Flag::type flags = renderer::Flag::ViewportBackground;;
            if (settings.get_general_settings()->flip_y) { flags |= renderer::Flag::FlipYAxis; }
            const bool only_background = !windows.shape_control || !draw_commands_ptr;

            // Render
            const Canvas<float> fb_viewport_canvas(cast<scalar, float>(windows.viewport->get_viewport_canvas()), framebuffer_scale);
            if (only_background)
            {
                draw_2d_renderer->render_viewport_background(fb_viewport_canvas);
            }
            else
            {
                assert(draw_commands_ptr);
                bool new_cbp_segmentation = false;
                const DrawCommands<scalar>& transformed_draw_commands = cbp_segmentation.convert_cbps(*draw_commands_ptr, fb_viewport_canvas, geometry_has_changed, new_cbp_segmentation);
                const bool update_buffers = geometry_has_changed || new_cbp_segmentation;
                update_opengl_draw_list<scalar>(draw_2d_renderer->draw_list(), transformed_draw_commands, update_buffers, drawing_options);
                stable_sort_draw_commands(draw_2d_renderer->draw_list());
                draw_2d_renderer->render(fb_viewport_canvas, flags);
            }
        }

        // ImGui rendering (always on top of the viewport rendering)
        dear_imgui_context.render();

        // End frame
        glfwSwapBuffers(glfw_context.window());
    }

    return EXIT_SUCCESS;
}

