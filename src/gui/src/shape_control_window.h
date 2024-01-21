#pragma once

#include "canvas.h"
#include "draw_command.h"
#include "renderer.h"
#include "settings.h"
#include "viewport_window.h"
#include "window_layout.h"

#include <dt/dt_impl.h>
#include <shapes/bounding_box.h>
#include <shapes/io.h>
#include <shapes/point.h>
#include <shapes/point_cloud.h>
#include <shapes/sampling_interface.h>
#include <shapes/shapes.h>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


class Settings;

class ShapeWindow
{
public:
    using scalar = ViewportWindow::scalar;
    using Key = typename ViewportWindow::Key;
    using DrawCommandListKVP = std::pair<Key, DrawCommands<scalar>>;
    using DrawCommandLists = std::vector<DrawCommandListKVP>;

    ShapeWindow(
        std::string_view name,
        shapes::io::ShapeAggregate<scalar>&& shapes,
        ViewportWindow& viewport_window);
    ~ShapeWindow();
    ShapeWindow(const ShapeWindow&) = delete;
    ShapeWindow& operator=(const ShapeWindow&) = delete;
    ShapeWindow(ShapeWindow&&) = delete;
    ShapeWindow& operator=(ShapeWindow&&) = delete;

    void visit(bool& can_be_erased, const Settings& settings, const WindowLayout& win_pos_sz, bool& input_has_changed);

    const DrawCommandLists& get_draw_command_lists() const;

    shapes::io::ShapeAggregate<scalar> get_triangulation_input_aggregate() const;
    shapes::io::ShapeAggregate<scalar> get_tab_aggregate(const Key& selected_tab) const;

    void add_steiner_point(const shapes::Point2d<scalar>& pt);

private:
    struct ShapeControl
    {
        struct PrimitiveData
        {
            PrimitiveData(std::size_t nb, const renderer::ColorData& color) : nb(nb), color(color), draw(true) {}
            std::size_t nb;
            renderer::ColorData color;
            bool draw;
        };
        explicit ShapeControl(shapes::AllShapes<scalar>&& shape);
        ShapeControl(const ShapeControl& shape_control);
        ShapeControl& operator=(const ShapeControl& shape_control);

        void update(shapes::AllShapes<scalar>&& rep_shape);
        DrawCommand<scalar> to_draw_command(const Settings& settings) const;

        bool active;
        bool force_inactive;
        bool highlight;
        float latest_computation_time_ms;
        PrimitiveData vertices;
        PrimitiveData edges;
        PrimitiveData faces;
        shapes::AllShapes<scalar> shape;
        std::string descr;
        std::unique_ptr<shapes::UniformSamplingInterface2d<scalar>> sampler;
        float req_sampling_length;
        ShapeControl* sampled_shape;
    };
    using ShapeControlSmartPtr = std::unique_ptr<ShapeControl>;
    using ShapeControlPtrs = std::vector<const ShapeControl*>;
    using ShapeControlListKVP = std::pair<Key, ShapeControlPtrs>;
    using ShapeControlLists = std::vector<ShapeControlListKVP>;

    struct TriangulationOutput
    {
        ShapeControlSmartPtr delaunay_triangulation;
    };

    struct ProximityGraphs
    {
        ShapeControlSmartPtr nn_graph;
        ShapeControlSmartPtr mst_graph;
        ShapeControlSmartPtr rng_graph;
        ShapeControlSmartPtr gg_graph;
        ShapeControlSmartPtr dt_graph;
    };

    void init_bounding_box();
    ShapeControlPtrs get_active_input_shapes() const;
    void recompute_triangulations(delaunay::TriangulationPolicy policy, const stdutils::io::ErrorHandler& err_handler);
    shapes::PointCloud2d<scalar> compute_input_point_cloud(const stdutils::io::ErrorHandler& err_handler);
    void compute_proximity_graphs(const stdutils::io::ErrorHandler& err_handler);
    void map_shape_controls_by_tabs(bool flag_include_proxiity_graphs);
    void build_draw_lists(const Settings& settings);
    ShapeControl* allocate_new_sampled_shape(const ShapeControl& parent, shapes::AllShapes<scalar>&& shape);
    void delete_sampled_shape(ShapeControl** sc);
    static bool active_button(std::string_view subid, unsigned int idx, ShapeControl& shape_control);
    static constexpr bool ALLOW_SAMPLING = true;
    static constexpr bool ALLOW_TINKERING = true;
    void shape_list_menu(ShapeControl& shape_control, unsigned int idx, bool allow_sampling, bool allow_tinkering, bool& in_out_trash, bool& input_has_changed);

    const std::string m_title;
    std::vector<ShapeControl> m_input_shape_controls;
    std::vector<ShapeControlSmartPtr> m_sampled_shape_controls;
    ShapeControl m_steiner_shape_control;
    std::optional<shapes::Point2d<scalar>> m_new_steiner_pt;
    delaunay::TriangulationPolicy m_triangulation_policy;
    std::map<std::string, TriangulationOutput> m_triangulation_shape_controls;
    std::vector<ShapeControl> m_triangulation_constraint_edges;
    ProximityGraphs m_proximity_graphs_controls;
    shapes::BoundingBox2d<scalar> m_geometry_bounding_box;
    ShapeControlLists m_shape_control_lists;
    DrawCommandLists m_draw_command_lists;
    Settings::General m_prev_general_settings;
    bool m_first_visit;
};
