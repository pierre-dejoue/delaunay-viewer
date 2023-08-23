#pragma once

#include "canvas.h"
#include "draw_command.h"
#include "renderer.h"
#include "settings.h"
#include "viewport_window.h"

#include <dt/dt_impl.h>
#include <shapes/bounding_box.h>
#include <shapes/sampling.h>
#include <shapes/shapes.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


class Settings;

class ShapeWindow
{
public:
    using scalar = ViewportWindow::scalar;
    struct ShapeControl
    {
        ShapeControl(shapes::AllShapes<scalar>&& shape);

        void update(shapes::AllShapes<scalar>&& rep_shape);
        DrawCommand<scalar> to_draw_command(const Settings& settings, bool constraint_edges = false) const;

        std::size_t nb_vertices;
        std::size_t nb_edges;
        std::size_t nb_faces;
        bool active;
        bool force_inactive;
        bool highlight;
        renderer::ColorData point_color;
        renderer::ColorData edge_color;
        renderer::ColorData face_color;
        float latest_computation_time_ms;
        shapes::AllShapes<scalar> shape;
        std::unique_ptr<shapes::UniformSamplingInterface2d<scalar>> sampler;
        float sampling_length;
        ShapeControl* sampled_shape;
    };

    using Key = typename ViewportWindow::Key;
    using DrawCommandListKVP = std::pair<Key, DrawCommands<scalar>>;
    using DrawCommandLists = std::vector<DrawCommandListKVP>;

    ShapeWindow(
        std::string_view name,
        ScreenPos initial_pos,
        std::vector<shapes::AllShapes<scalar>>&& shapes,
        ViewportWindow& viewport_window);
    ~ShapeWindow();
    ShapeWindow(const ShapeWindow&) = delete;
    ShapeWindow& operator=(const ShapeWindow&) = delete;

    void visit(bool& can_be_erased, const Settings& settings, bool& input_has_changed);

    const DrawCommandLists& get_draw_command_lists() const;

private:
    void init_bounding_box();
    void recompute_triangulations();
    void build_draw_lists(const Settings& settings);
    ShapeControl* allocate_new_sampled_shape(const ShapeControl& parent, shapes::AllShapes<scalar>&& shape);
    void delete_sampled_shape(ShapeControl** sc);
    static constexpr bool ALLOW_SAMPLING = true;
    void shape_list_menu(ShapeControl& shape_control, unsigned int idx, bool allow_sampling, bool& input_has_changed);

    const std::string m_title;
    ScreenPos m_initial_pos;
    std::vector<ShapeControl> m_input_shape_controls;
    std::vector<std::unique_ptr<ShapeControl>> m_sampled_shape_controls;
    std::map<std::string, ShapeControl> m_triangulation_shape_controls;
    shapes::BoundingBox2d<scalar> m_geometry_bounding_box;
    DrawCommandLists m_draw_command_lists;
    bool m_first_visit;
};
