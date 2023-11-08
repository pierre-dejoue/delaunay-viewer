#include "shape_control_window.h"

#include "draw_shape.h"
#include "imgui_helpers.h"
#include "settings.h"

#include <dt/dt_interface.h>
#include <dt/proximity_graphs.h>
#include <shapes/bounding_box_algos.h>
#include <shapes/sampling.h>
#include <stdutils/chrono.h>
#include <stdutils/io.h>
#include <stdutils/macros.h>
#include <stdutils/visit.h>

#include <imgui_wrap.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <utility>
#include <variant>


namespace
{
    const char* INPUT_TAB_NAME = "Input";

    constexpr ImU32 VertexColor_Default     = IM_COL32(20, 90, 116, 255);
    constexpr ImU32 VertexColor_Highlighted = IM_COL32(230, 230, 255, 255);

    constexpr ImU32 EdgeColor_Default       = IM_COL32(91, 94, 137, 255);
    constexpr ImU32 EdgeColor_Constraint    = IM_COL32(222, 91, 94, 255);
    constexpr ImU32 EdgeColor_Highlighted   = IM_COL32(190, 230, 255, 255);
    constexpr ImU32 EdgeColor_Proximity     = IM_COL32(160, 170, 255, 255);

    constexpr ImU32 FaceColor_Default       = IM_COL32(80, 82, 105, 255);
    constexpr ImU32 FaceColor_Highlighted   = IM_COL32(170, 210, 255, 255);

    const auto VertexColor_Float_Default = to_float_color(VertexColor_Default);
    const auto EdgeColor_Float_Default =   to_float_color(EdgeColor_Default);
    const auto FaceColor_Float_Default =   to_float_color(FaceColor_Default);

    renderer::ColorData get_vertex_color(bool highlight, const renderer::ColorData& def)
    {
        return (highlight ? to_float_color(VertexColor_Highlighted) : def);
    }

    renderer::ColorData get_edge_color(bool highlight, bool constraint_edges, const renderer::ColorData& def)
    {
        if (highlight)
            return to_float_color(EdgeColor_Highlighted);
        else if (constraint_edges)
            return to_float_color(EdgeColor_Constraint);
        else
            return def;
    }

    renderer::ColorData get_face_color(bool highlight, const Settings::Surface& surface_settings, const renderer::ColorData& def)
    {
        const float alpha = std::clamp(surface_settings.alpha, 0.f, 1.f);
        auto base_color = (highlight ? to_float_color(FaceColor_Highlighted) : def);
        base_color[3] *= alpha;
        return base_color;
    }

    renderer::ColorData& luminosity(renderer::ColorData& color, float ratio)
    {
        color[0] = std::min(color[0] * ratio, 1.f);
        color[1] = std::min(color[1] * ratio, 1.f);
        color[2] = std::min(color[2] * ratio, 1.f);
        return color;
    }

    template <typename F>
    shapes::AllShapes<F> swap_shape_type_point_cloud_and_point_path(shapes::AllShapes<F>&& shape)
    {
        shapes::AllShapes<F> result;
        std::visit(stdutils::Overloaded {
            [&result](shapes::PointCloud2d<F>& pc)          { auto& pp = result.template emplace<shapes::PointPath2d<F>>();  pp.vertices = std::move(pc.vertices); pp.closed = (pp.vertices.size() > 2); },
            [&result](shapes::PointCloud3d<F>& pc)          { auto& pp = result.template emplace<shapes::PointPath3d<F>>();  pp.vertices = std::move(pc.vertices); pp.closed = (pp.vertices.size() > 2); },
            [&result](shapes::PointPath2d<F>& pp)           { auto& pc = result.template emplace<shapes::PointCloud2d<F>>(); pc.vertices = std::move(pp.vertices); },
            [&result](shapes::PointPath3d<F>& pp)           { auto& pc = result.template emplace<shapes::PointCloud3d<F>>(); pc.vertices = std::move(pp.vertices); },
            [&result](auto& s)                              { assert(0); result = std::move(s); }
        }, shape);
        return result;
    }

} // Anonymous namespace

ShapeWindow::ShapeControl::ShapeControl(shapes::AllShapes<scalar>&& shape)
    : nb_vertices(shapes::nb_vertices(shape))
    , nb_edges(shapes::nb_edges(shape))
    , nb_faces(shapes::nb_faces(shape))
    , active(true)
    , force_inactive(false)
    , highlight(false)
    , point_color(VertexColor_Float_Default)
    , edge_color(EdgeColor_Float_Default)
    , face_color(FaceColor_Float_Default)
    , latest_computation_time_ms(0.f)
    , shape(std::move(shape))
    , sampler()
    , req_sampling_length(1.f)
    , sampled_shape(nullptr)
{ }

void ShapeWindow::ShapeControl::update(shapes::AllShapes<scalar>&& rep_shape)
{
    shape = std::move(rep_shape);
    nb_vertices = shapes::nb_vertices(shape);
    nb_edges = shapes::nb_edges(shape);
    nb_faces = shapes::nb_faces(shape);
    // 'active', 'hightlight' remain as-is
}

DrawCommand<ShapeWindow::scalar> ShapeWindow::ShapeControl::to_draw_command(const Settings& settings, bool constraint_edges) const
{
    DrawCommand<ShapeWindow::scalar> result(shape);
    result.point_color = get_vertex_color(highlight, point_color);
    result.edge_color = get_edge_color(highlight, constraint_edges, edge_color);
    result.face_color = get_face_color(highlight, settings.read_surface_settings(), face_color);
    return result;
}

ShapeWindow::ShapeWindow(
        std::string_view name,
        std::vector<shapes::AllShapes<scalar>>&& shapes,
        ViewportWindow& viewport_window)
    : m_title(std::string(name) + " Controls")
    , m_input_shape_controls()
    , m_sampled_shape_controls()
    , m_steiner_shape_control(shapes::PointCloud2d<scalar>())
    , m_new_steiner_pt()
    , m_triangulation_policy(delaunay::TriangulationPolicy::PointCloud)
    , m_triangulation_shape_controls()
    , m_proximity_graphs_controls()
    , m_geometry_bounding_box()
    , m_draw_command_lists()
    , m_prev_general_settings()
    , m_first_visit(true)
{
    m_input_shape_controls.reserve(shapes.size());
    for (auto& shape : shapes)
        m_input_shape_controls.emplace_back(std::move(shape));
    shapes.clear();
    init_bounding_box();
    viewport_window.set_geometry_bounding_box(m_geometry_bounding_box);
}

ShapeWindow::~ShapeWindow() = default;

void ShapeWindow::init_bounding_box()
{
    for (const auto& shape_control : m_input_shape_controls)
        std::visit(stdutils::Overloaded {
            [this](const shapes::PointCloud2d<scalar>& s) { m_geometry_bounding_box.merge(shapes::fast_bounding_box(s)); },
            [this](const shapes::PointPath2d<scalar>& s) { m_geometry_bounding_box.merge(shapes::fast_bounding_box(s)); },
            [this](const shapes::CubicBezierPath2d<scalar>& s) { m_geometry_bounding_box.merge(shapes::fast_bounding_box(s)); },
            [](const auto&) { assert(0); }
        }, shape_control.shape);
    shapes::ensure_min_extent(m_geometry_bounding_box);
}

// The input for the triangulation algorithm
std::vector<const ShapeWindow::ShapeControl*> ShapeWindow::get_active_shapes() const
{
    std::vector<const ShapeControl*> active_shapes;
    for (const auto& shape_control : m_input_shape_controls)
    {
        if (shape_control.active)
            active_shapes.emplace_back(&shape_control);
    }
    for (const auto& shape_control_ptr : m_sampled_shape_controls)
    {
        assert(shape_control_ptr);
        if (shape_control_ptr->active)
            active_shapes.emplace_back(shape_control_ptr.get());
    }
    if (m_steiner_shape_control.active && m_steiner_shape_control.nb_vertices > 0)
    {
        active_shapes.emplace_back(&m_steiner_shape_control);
    }
    return active_shapes;
}

void ShapeWindow::recompute_triangulations(delaunay::TriangulationPolicy policy, const stdutils::io::ErrorHandler& err_handler)
{
    std::chrono::duration<float, std::milli> duration;
    shapes::Triangles2d<scalar> triangulation;
    for (const auto& algo : delaunay::get_impl_list<scalar>().algos)
    {
        // Setup triangulation
        m_triangulation_shape_controls.try_emplace(algo.name);
        auto triangulation_algo = delaunay::get_impl(algo, &err_handler);
        assert(triangulation_algo);
        bool first_path = true;
        const auto active_shapes = get_active_shapes();
        for (const auto* shape_control_ptr : active_shapes)
        {
            std::visit(stdutils::Overloaded {
                [&triangulation_algo](const shapes::PointCloud2d<scalar>& pc) { triangulation_algo->add_steiner(pc); },
                [&triangulation_algo, &first_path](const shapes::PointPath2d<scalar>& pp) {
                    if (first_path) { triangulation_algo->add_path(pp); first_path = false; }
                    else { triangulation_algo->add_hole(pp); }
                },
                [](const shapes::CubicBezierPath2d<scalar>&) { /* Skip */ },
                [](const auto&) { assert(0); }
            }, shape_control_ptr->shape);
        }

        // Triangulate
        {
            stdutils::chrono::DurationMeas meas(duration);
            triangulation = triangulation_algo->triangulate(policy);
        }

        // Update or create the output shape controls
        if (m_triangulation_shape_controls.at(algo.name).delaunay_triangulation)
            m_triangulation_shape_controls.at(algo.name).delaunay_triangulation->update(std::move(triangulation));
        else
            m_triangulation_shape_controls.at(algo.name).delaunay_triangulation = std::make_unique<ShapeControl>(std::move(triangulation));
        assert(m_triangulation_shape_controls.at(algo.name).delaunay_triangulation);
        m_triangulation_shape_controls.at(algo.name).delaunay_triangulation->latest_computation_time_ms = duration.count();
    }
}

void ShapeWindow::compute_proximity_graphs(const stdutils::io::ErrorHandler& err_handler)
{
    const auto active_shapes = get_active_shapes();
    shapes::PointCloud2d<scalar> proximity_point_cloud;
    for (const auto* shape_control_ptr : active_shapes)
    {
        std::visit(stdutils::Overloaded {
            [&proximity_point_cloud](const shapes::PointCloud2d<scalar>& pc) { proximity_point_cloud.vertices.insert(proximity_point_cloud.vertices.end(), pc.vertices.cbegin(), pc.vertices.cend()); },
            [&proximity_point_cloud](const shapes::PointPath2d<scalar>& pp)  { proximity_point_cloud.vertices.insert(proximity_point_cloud.vertices.end(), pp.vertices.cbegin(), pp.vertices.cend()); },
            [](const shapes::CubicBezierPath2d<scalar>&) { /* Skip */ },
            [](const auto&) { assert(0); }
        }, shape_control_ptr->shape);
    }

    auto edge_color = to_float_color(EdgeColor_Proximity);
    float lum_ratio = 0.75f;

    // NN
    auto nn = delaunay::nearest_neighbor(proximity_point_cloud, err_handler);
    if (m_proximity_graphs_controls.nn_graph)
    {
        m_proximity_graphs_controls.nn_graph->update(std::move(nn));
    }
    else
    {
        m_proximity_graphs_controls.nn_graph = std::make_unique<ShapeControl>(std::move(nn));
        m_proximity_graphs_controls.nn_graph->edge_color = edge_color;
    }
    luminosity(edge_color, lum_ratio);

    // MST
    auto mst = delaunay::minimum_spanning_tree(proximity_point_cloud, err_handler);
    if (m_proximity_graphs_controls.mst_graph)
    {
        m_proximity_graphs_controls.mst_graph->update(std::move(mst));
    }
    else
    {
        m_proximity_graphs_controls.mst_graph = std::make_unique<ShapeControl>(std::move(mst));
        m_proximity_graphs_controls.mst_graph->edge_color = edge_color;
    }
    luminosity(edge_color, lum_ratio);

    // RNG
    auto rng = delaunay::relative_neighborhood_graph(proximity_point_cloud, err_handler);
    if (m_proximity_graphs_controls.rng_graph)
    {
        m_proximity_graphs_controls.rng_graph->update(std::move(rng));
    }
    else
    {
        m_proximity_graphs_controls.rng_graph = std::make_unique<ShapeControl>(std::move(rng));
        m_proximity_graphs_controls.rng_graph->edge_color = edge_color;
    }
    luminosity(edge_color, lum_ratio);

    // GG
    auto gg = delaunay::gabriel_graph(proximity_point_cloud, err_handler);
    if (m_proximity_graphs_controls.gg_graph)
    {
        m_proximity_graphs_controls.gg_graph->update(std::move(gg));
    }
    else
    {
        m_proximity_graphs_controls.gg_graph = std::make_unique<ShapeControl>(std::move(gg));
        m_proximity_graphs_controls.gg_graph->edge_color = edge_color;
    }
    luminosity(edge_color, lum_ratio);

    // DT
    auto dt = delaunay::delaunay_triangulation(proximity_point_cloud, err_handler);
    if (m_proximity_graphs_controls.dt_graph)
    {
        m_proximity_graphs_controls.dt_graph->update(std::move(dt));
    }
    else
    {
        m_proximity_graphs_controls.dt_graph = std::make_unique<ShapeControl>(std::move(dt));
        m_proximity_graphs_controls.dt_graph->edge_color = edge_color;
    }
}

void ShapeWindow::build_draw_lists(const Settings& settings)
{
    m_draw_command_lists.clear();
    {
        auto& draw_command_list = m_draw_command_lists.emplace_back(INPUT_TAB_NAME, DrawCommands<scalar>()).second;
        for (const auto& shape_control : m_input_shape_controls)
        {
            if (shape_control.active)
                draw_command_list.emplace_back(shape_control.to_draw_command(settings));
        }
        for (const auto& shape_control_ptr : m_sampled_shape_controls)
        {
            assert(shape_control_ptr);
            if (shape_control_ptr->active)
                draw_command_list.emplace_back(shape_control_ptr->to_draw_command(settings));
        }
        if(m_steiner_shape_control.active && m_steiner_shape_control.nb_vertices > 0)
        {
            draw_command_list.emplace_back(m_steiner_shape_control.to_draw_command(settings));
        }
    }
    const auto triangulation_policy = settings.read_general_settings().cdt ? delaunay::TriangulationPolicy::CDT :  delaunay::TriangulationPolicy::PointCloud;
    constexpr bool CONSTRAINED_EDGES = true;
    for (const auto& [algo_name, triangulation_output] : m_triangulation_shape_controls)
    {
        if (!triangulation_output.delaunay_triangulation)
            continue;
        const auto& shape_control_delaunay = *triangulation_output.delaunay_triangulation;
        auto& draw_command_list = m_draw_command_lists.emplace_back(algo_name, DrawCommands<scalar>()).second;
        draw_command_list.emplace_back(shape_control_delaunay.to_draw_command(settings));
        if (triangulation_policy == delaunay::TriangulationPolicy::CDT)
        {
            for (const auto& shape_control : m_input_shape_controls)
            {
                if (shape_control.active && !shapes::is_bezier_path(shape_control.shape))
                {
                    draw_command_list.emplace_back(shape_control.to_draw_command(settings, CONSTRAINED_EDGES));
                }
            }
            for (const auto& shape_control_ptr : m_sampled_shape_controls)
            {
                if (shape_control_ptr->active)
                {
                    assert(!shapes::is_bezier_path(shape_control_ptr->shape));
                    draw_command_list.emplace_back(shape_control_ptr->to_draw_command(settings, CONSTRAINED_EDGES));
                }
            }
        }
        if(m_steiner_shape_control.active && m_steiner_shape_control.nb_vertices > 0)
        {
            draw_command_list.emplace_back(m_steiner_shape_control.to_draw_command(settings));
        }
    }
    // Crust
    // Proximity Graphs
    if (settings.read_general_settings().proximity_graphs)
    {
        auto& draw_command_list = m_draw_command_lists.emplace_back("Proximity Graphs", DrawCommands<scalar>()).second;
        std::vector<const ShapeControl*> proxi_graphs = {
            m_proximity_graphs_controls.dt_graph.get(),
            m_proximity_graphs_controls.gg_graph.get(),
            m_proximity_graphs_controls.rng_graph.get(),
            m_proximity_graphs_controls.mst_graph.get(),
            m_proximity_graphs_controls.nn_graph.get(),
        };
        for (const auto* proxi_graph: proxi_graphs)
        {
            if (proxi_graph && proxi_graph->active)
            {
                draw_command_list.emplace_back(proxi_graph->to_draw_command(settings));
            }
        }
    }
    else
    {
        m_draw_command_lists.emplace_back("Proximity Graphs", DrawCommands<scalar>());
    }
}

shapes::io::ShapeAggregate<ShapeWindow::scalar> ShapeWindow::get_triangulation_input_aggregate() const
{
    shapes::io::ShapeAggregate<ShapeWindow::scalar> result;
    for (const auto& shape_control : m_input_shape_controls)
    {
        if (shape_control.active)
            result.emplace_back(shape_control.shape);
    }
    for (const auto& shape_control_ptr : m_sampled_shape_controls)
    {
        assert(shape_control_ptr);
        if (shape_control_ptr->active)
            result.emplace_back(shape_control_ptr->shape);
    }
    if(m_steiner_shape_control.active && m_steiner_shape_control.nb_vertices > 0)
    {
        result.emplace_back(m_steiner_shape_control.shape);
    }
    return result;
}

ShapeWindow::ShapeControl* ShapeWindow::allocate_new_sampled_shape(const ShapeControl& parent, shapes::AllShapes<scalar>&& shape)
{
    const auto& new_shape = m_sampled_shape_controls.emplace_back(std::make_unique<ShapeControl>(std::move(shape)));

    // Inherit colors from parent
    new_shape->point_color = parent.point_color;
    new_shape->edge_color = parent.edge_color;
    new_shape->face_color = parent.face_color;

    return new_shape.get();
}

void ShapeWindow::delete_sampled_shape(ShapeControl** sc)
{
    const auto sc_it = std::find_if(std::cbegin(m_sampled_shape_controls), std::cend(m_sampled_shape_controls), [sc](const auto& elt) { return elt.get() == *sc; });
    assert(sc_it != std::cend(m_sampled_shape_controls));
    m_sampled_shape_controls.erase(sc_it);
    *sc = nullptr;
}

bool ShapeWindow::active_button(std::string_view subid, unsigned int idx, ShapeControl& shape_control)
{
    bool geometry_has_changed = false;
    std::stringstream active_button;
    active_button << "Active#" << subid << idx;
    float hue = shape_control.active ? 0.3f : 0.f;
    ImGui::PushID(active_button.str().c_str());
    ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));
    ImGui::PushStyleColor(ImGuiCol_Text, shape_control.active ? ImVec4(1.f, 1.f, 1.f, 1.f) : ImVec4(0.4f, 0.4f, 0.4f, 1.f));
    const bool pressed = ImGui::Button("Active");
    ImGui::PopStyleColor(4);
    ImGui::PopID();
    if (pressed && !shape_control.force_inactive)
    {
        shape_control.active = !shape_control.active;
        geometry_has_changed = true;
    }
    return geometry_has_changed;
}

void ShapeWindow::shape_list_menu(ShapeControl& shape_control, unsigned int idx, bool allow_sampling, bool allow_tinkering, bool& in_out_trash, bool& geometry_has_changed)
{
    std::stringstream label;
    label << "Shape #" << idx;
    const bool is_open = ImGui::TreeNode(label.str().c_str());
    shape_control.highlight = ImGui::IsItemHovered();
    if (!is_open)
    {
        in_out_trash = false;
        return;
    }

    // Active button
    geometry_has_changed |= active_button("input", idx, shape_control);

    // Trash button
    if (in_out_trash)
    {
        ImGui::SameLine(0, 30);
        std::stringstream trash_button;
        trash_button << "Trash#" << idx;
        ImGui::PushID(trash_button.str().c_str());
        in_out_trash = ImGui::Button("Trash");
        ImGui::PopID();
    }

    // Tinker with the shape: Point Cloud <-> Point Path, Open <-> Closed
    if (allow_tinkering && shape_control.sampled_shape == nullptr && (shapes::is_point_cloud(shape_control.shape) || shapes::is_point_path(shape_control.shape)))
    {
        ImGui::SameLine(0, 30);
        std::stringstream path_checkbox;
        path_checkbox << "Path#" << idx;
        ImGui::PushID(path_checkbox.str().c_str());
        bool is_path = shapes::is_point_path(shape_control.shape);
        const bool swap_shape_type = ImGui::Checkbox("Path", &is_path);
        ImGui::PopID();
        if (swap_shape_type)
        {
            auto temp_shape = swap_shape_type_point_cloud_and_point_path(std::move(shape_control.shape));
            shape_control.update(std::move(temp_shape));
            geometry_has_changed = true;
        }
        else if (is_path)
        {
            ImGui::SameLine(0);
            std::stringstream topo_checkbox;
            topo_checkbox << "Closed#" << idx;
            ImGui::PushID(topo_checkbox.str().c_str());
            bool is_closed = shapes::is_closed(shape_control.shape);
            const bool swap_topo = ImGui::Checkbox("Closed", &is_closed);
            ImGui::PopID();
            if(swap_topo)
            {
                shapes::flip_open_closed(shape_control.shape);
                shape_control.nb_edges = shapes::nb_edges(shape_control.shape);
                geometry_has_changed = true;
            }
        }
    }

    // Color pickers
    ImGui::ColorEdit4("Point color", shape_control.point_color.data(), ImGuiColorEditFlags_NoInputs);
    if (shapes::has_edges(shape_control.shape)) { ImGui::SameLine(); ImGui::ColorEdit4("Edge color", shape_control.edge_color.data(), ImGuiColorEditFlags_NoInputs); }
    if (shapes::has_faces(shape_control.shape)) { ImGui::SameLine(); ImGui::ColorEdit4("Face color", shape_control.face_color.data(), ImGuiColorEditFlags_NoInputs); }

    // Info
    ImGui::Text("Nb vertices: %ld, nb edges: %ld", shape_control.nb_vertices, shape_control.nb_edges);

    // Sampling
    if (allow_sampling && (shapes::is_bezier_path(shape_control.shape) || shapes::is_point_path(shape_control.shape)))
    {
        std::stringstream sample_checkbox;
        sample_checkbox << "Sample##" << idx;
        bool is_sampled = (shape_control.sampled_shape != nullptr);
        ImGui::Checkbox(sample_checkbox.str().c_str(), &is_sampled);
        if (is_sampled && !shape_control.sampled_shape)
        {
            shape_control.sampled_shape = allocate_new_sampled_shape(shape_control, shapes::trivial_sampling(shape_control.shape));
            if (std::holds_alternative<shapes::CubicBezierPath2d<scalar>>(shape_control.shape))
            {
                const auto& cbp = std::get<shapes::CubicBezierPath2d<scalar>>(shape_control.shape);
                shape_control.sampler = std::make_unique<shapes::UniformSamplingCubicBezier2d<scalar>>(cbp);
                shape_control.req_sampling_length = static_cast<float>(shape_control.sampler->max_segment_length());
            }
            else if (std::holds_alternative<shapes::PointPath2d<scalar>>(shape_control.shape))
            {
                const auto& pp = std::get<shapes::PointPath2d<scalar>>(shape_control.shape);
                shape_control.active = false;
                shape_control.force_inactive = true;
                shape_control.sampler = std::make_unique<shapes::UniformSamplingPointPath2d<scalar>>(pp);
                shape_control.req_sampling_length = static_cast<float>(shape_control.sampler->max_segment_length());
            }
            geometry_has_changed = true;
        }
        if (!is_sampled && shape_control.sampled_shape)
        {
            shape_control.sampler.reset();
            delete_sampled_shape(&shape_control.sampled_shape);
            shape_control.force_inactive = false;
            geometry_has_changed = true;
        }
        if (shape_control.sampler)
        {
            std::stringstream sampling_length_id;
            sampling_length_id << "Sampling length##" << idx;
            assert(is_sampled);
            const float max = 1.05f * static_cast<float>(shape_control.sampler->max_segment_length());
            const float min = max / 1000.f;
            float new_sampling_length = shape_control.req_sampling_length;
            ImGui::SliderFloat(sampling_length_id .str().c_str(), &new_sampling_length, min, max, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
            if (new_sampling_length != shape_control.req_sampling_length)
            {
                shape_control.req_sampling_length = new_sampling_length;
                shape_control.sampled_shape->update(shapes::AllShapes<scalar>(shape_control.sampler->sample(static_cast<scalar>(new_sampling_length))));
                geometry_has_changed = true;
            }
        }
    }
    ImGui::TreePop();
}

void ShapeWindow::visit(bool& can_be_erased, const Settings& settings, const WindowLayout& win_pos_sz, bool& geometry_has_changed)
{
    geometry_has_changed = m_first_visit;
    m_first_visit = false;

    stdutils::io::ErrorHandler err_handler = [](stdutils::io::SeverityCode code, std::string_view msg) { std::cout << stdutils::io::str_severity_code(code) << ": " << msg << std::endl; };

    const auto triangulation_policy = settings.read_general_settings().cdt ? delaunay::TriangulationPolicy::CDT :  delaunay::TriangulationPolicy::PointCloud;
    geometry_has_changed |= triangulation_policy != m_triangulation_policy;

    const bool display_proximity_graphs = settings.read_general_settings().proximity_graphs;
    if (display_proximity_graphs != m_prev_general_settings.proximity_graphs)
        geometry_has_changed = true;
    m_prev_general_settings = settings.read_general_settings();

    ImGui::SetNextWindowPosAndSize(win_pos_sz);
    constexpr ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoCollapse
                                         | ImGuiWindowFlags_NoMove
                                         | ImGuiWindowFlags_NoResize
                                         | ImGuiWindowFlags_NoSavedSettings;;

    bool is_window_open = true;
    if (!ImGui::Begin(m_title.c_str(), &is_window_open, win_flags))
    {
        // Collapsed
        can_be_erased = !is_window_open;
        ImGui::End();
        return;
    }
    can_be_erased = !is_window_open;

    // Global bounding box
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Input bounding box"))
    {
        if (ImGui::BeginTable("bounding_box_table", 3, ImGuiTableFlags_None))
        {
            {
                ImGui::TableNextRow();
                const auto tl_corner = m_geometry_bounding_box.min();
                ImGui::TableNextColumn();
                ImGui::Text("Top-left corner");
                ImGui::TableNextColumn();
                ImGui::Text("%0.3g", tl_corner.x);
                ImGui::TableNextColumn();
                ImGui::Text("%0.3g", tl_corner.y);
            }
            {
                ImGui::TableNextRow();
                const auto br_corner = m_geometry_bounding_box.max();
                ImGui::TableNextColumn();
                ImGui::Text("Bottom-right corner");
                ImGui::TableNextColumn();
                ImGui::Text("%0.3g", br_corner.x);
                ImGui::TableNextColumn();
                ImGui::Text("%0.3g", br_corner.y);
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // Iterate all input/samples shapes
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Input shapes"))
    {
        unsigned int shape_idx = 1;
        for (auto& shape_control : m_input_shape_controls)
        {
            bool trash = false;
            shape_list_menu(shape_control, shape_idx++, ALLOW_SAMPLING, ALLOW_TINKERING, trash, geometry_has_changed);
        }
        ImGui::TreePop();
    }
    else
    {
        for (auto& shape_control : m_input_shape_controls)
        {
            shape_control.highlight = false;
        }
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Sampled shapes"))
    {
        unsigned int shape_idx = 1;
        for (auto& shape_control_ptr : m_sampled_shape_controls)
        {
            assert(shape_control_ptr);
            bool trash = false;
            shape_list_menu(*shape_control_ptr, shape_idx++, !ALLOW_SAMPLING, ALLOW_TINKERING, trash, geometry_has_changed);
        }
        ImGui::TreePop();
    }
    else
    {
        for (auto& shape_control_ptr : m_sampled_shape_controls)
        {
            assert(shape_control_ptr);
            shape_control_ptr->highlight = false;
        }
    }

    if (m_new_steiner_pt.has_value())
    {
        auto new_pt = m_new_steiner_pt.value();     // Tried to take the contained value with *std::move(m_new_steiner_pt), it didn't work.
        m_new_steiner_pt.reset();
        auto& pc = std::get<shapes::PointCloud2d<scalar>>(m_steiner_shape_control.shape);
        const auto pt_it = std::find(std::cbegin(pc.vertices), std::cend(pc.vertices), new_pt);
        if (pt_it == std::cend(pc.vertices))
        {
            pc.vertices.emplace_back(std::move(new_pt));
            m_steiner_shape_control.nb_vertices = pc.vertices.size();
            geometry_has_changed = true;
        }
        else
        {
            err_handler(stdutils::io::Severity::INFO, "The new steiner point was skipped because it is a duplicate.");
        }
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Steiner points"))
    {
        const unsigned int shape_idx = 1;
        bool trash = true;
        shape_list_menu(m_steiner_shape_control, shape_idx, !ALLOW_SAMPLING, !ALLOW_TINKERING, trash, geometry_has_changed);
        if (trash)
        {
            m_steiner_shape_control.update(shapes::PointCloud2d<scalar>());
            geometry_has_changed = true;
        }
        ImGui::TreePop();
    }

    // Triangulate
    if (geometry_has_changed)
    {
        recompute_triangulations(triangulation_policy, err_handler);
        m_triangulation_policy = triangulation_policy;
        if (display_proximity_graphs)
            compute_proximity_graphs(err_handler);
    }

    // Triangulation shapes
    for (auto& [algo_name, triangulation_output] : m_triangulation_shape_controls)
    {
        if (!triangulation_output.delaunay_triangulation)
            continue;
        auto& triangulation_shape_control = *triangulation_output.delaunay_triangulation;
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode(algo_name.c_str()))
        {
            const bool is_open = ImGui::TreeNode("Shape #1");
            triangulation_shape_control.highlight = ImGui::IsItemHovered();
            if (is_open)
            {
                // Color pickers
                ImGui::ColorEdit4("Point color", triangulation_shape_control.point_color.data(), ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::ColorEdit4("Edge color", triangulation_shape_control.edge_color.data(), ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::ColorEdit4("Face color", triangulation_shape_control.face_color.data(), ImGuiColorEditFlags_NoInputs);

                // Info
                ImGui::Text("Nb vertices: %ld, nb edges: %ld, nb faces: %ld", triangulation_shape_control.nb_vertices, triangulation_shape_control.nb_edges, triangulation_shape_control.nb_faces);
                ImGui::Text("Computation time: %0.3g ms", static_cast<double>(triangulation_shape_control.latest_computation_time_ms));
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    // Proximity graphs' shapes
    if (display_proximity_graphs)
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Proximity Graphs"))
        {
            std::vector<std::pair<std::string, ShapeControl*>> proxi_graphs;
            if (m_proximity_graphs_controls.nn_graph) { proxi_graphs.emplace_back("NN", m_proximity_graphs_controls.nn_graph.get()); }
            if (m_proximity_graphs_controls.mst_graph) { proxi_graphs.emplace_back("MST", m_proximity_graphs_controls.mst_graph.get()); }
            if (m_proximity_graphs_controls.rng_graph) { proxi_graphs.emplace_back("RNG", m_proximity_graphs_controls.rng_graph.get()); }
            if (m_proximity_graphs_controls.gg_graph) { proxi_graphs.emplace_back("GG", m_proximity_graphs_controls.gg_graph.get()); }
            if (m_proximity_graphs_controls.dt_graph) { proxi_graphs.emplace_back("DT", m_proximity_graphs_controls.dt_graph.get()); }

            unsigned int idx = 0;
            for (const auto& [graph_name, shape_control] : proxi_graphs)
            {
                const bool is_open = ImGui::TreeNode(graph_name.c_str());
                // Do not set shape_control->highlight if the item is hovered. It is confusing due to the superposition of the proximity graphs
                if (is_open)
                {
                    // Active button
                    geometry_has_changed |= active_button("proximity", idx, *shape_control);

                    // Color pickers
                    ImGui::ColorEdit4("Point color", shape_control->point_color.data(), ImGuiColorEditFlags_NoInputs);
                    ImGui::SameLine();
                    ImGui::ColorEdit4("Edge color", shape_control->edge_color.data(), ImGuiColorEditFlags_NoInputs);

                    // Info
                    ImGui::Text("Nb vertices: %ld, nb edges: %ld", shape_control->nb_vertices, shape_control->nb_edges);
                    ImGui::TreePop();
                }
                idx++;
            }

            ImGui::TreePop();
        }
    }

    ImGui::End();

    build_draw_lists(settings);
}

const ShapeWindow::DrawCommandLists& ShapeWindow::get_draw_command_lists() const
{
    return m_draw_command_lists;
}

void ShapeWindow::add_steiner_point(const shapes::Point2d<scalar>& pt)
{
    assert(m_new_steiner_pt.has_value() == false);      // By design we de not receive more than one point per frame, so one optional is enough
    m_new_steiner_pt = pt;
}
