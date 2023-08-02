#pragma once

#include "shape_draw_window.h"

#include <dt/dt_impl.h>
#include <shapes/bounding_box.h>
#include <shapes/sampling.h>
#include <shapes/shapes.h>

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


class Settings;

class ShapeWindow
{
public:
    using scalar = double;
    struct ShapeControl
    {
        ShapeControl(shapes::AllShapes<scalar>&& shape);
        void update(shapes::AllShapes<scalar>&& rep_shape);
        std::size_t nb_vertices;
        std::size_t nb_edges;
        std::size_t nb_faces;
        bool active;
        bool highlight;
        bool constraint_edges;
        float latest_computation_time_ms;
        shapes::AllShapes<scalar> shape;
        std::unique_ptr<shapes::UniformSamplingCubicBezier2d<scalar>> sampler;
        float sampling_length;
        ShapeControl* sampled_shape;
    };

    ShapeWindow(
        std::vector<shapes::AllShapes<scalar>>&& shapes,
        std::string_view name);
    ~ShapeWindow();
    ShapeWindow(const ShapeWindow&) = delete;
    ShapeWindow& operator=(const ShapeWindow&) = delete;

    void visit(bool& can_be_erased, const Settings& settings);

private:
    void init_bounding_box();
    void recompute_triangulations();
    typename ShapeDrawWindow::DrawCommandLists build_draw_lists() const;
    ShapeControl* allocate_new_sampled_shape(shapes::AllShapes<scalar>&& shape);
    void delete_sampled_shape(ShapeControl** sc);
    static constexpr bool ALLOW_SAMPLING = true;
    void shape_list_menu(ShapeControl& shape_control, unsigned int idx, bool allow_sampling, bool& input_has_changed);

    std::vector<ShapeControl> m_input_shape_controls;
    std::vector<std::unique_ptr<ShapeControl>> m_sampled_shape_controls;
    std::vector<std::pair<std::string, ShapeControl>> m_triangulation_shape_controls;
    const std::string m_title;
    std::unique_ptr<ShapeDrawWindow> m_draw_window;
    shapes::BoundingBox2d<scalar> m_bounding_box;
};
