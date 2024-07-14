#pragma once

#include <stdutils/parameter.h>

#include <memory>

class Settings
{
public:
    struct GeneralLimits
    {
        stdutils::parameter::Limits<bool> flip_y;
        stdutils::parameter::Limits<bool> imgui_renderer;
        stdutils::parameter::Limits<bool> line_smooth;
        stdutils::parameter::Limits<bool> cdt;
        stdutils::parameter::Limits<bool> proximity_graphs;
    };
    struct General
    {
        bool flip_y;        // If false, the Y-axis is in the "up" direction.
        bool imgui_renderer;
        bool line_smooth;
        bool cdt;
        bool proximity_graphs;
    };
    struct PointLimits
    {
        stdutils::parameter::Limits<bool> show;
        stdutils::parameter::Limits<float> size;
    };
    struct Point
    {
        bool show;
        float size;
    };
    struct PathLimits
    {
        stdutils::parameter::Limits<bool> show;
        stdutils::parameter::Limits<float> width;
    };
    struct Path
    {
        bool show;
        float width;
    };
    struct SurfaceLimits
    {
        stdutils::parameter::Limits<bool> show;
        stdutils::parameter::Limits<float> alpha;
    };
    struct Surface
    {
        bool show;
        float alpha;
    };

public:
    Settings();

    // General settings
    General* get_general_settings();
    const General& read_general_settings() const;
    static const GeneralLimits& read_general_limits();

    // Point settings
    Point* get_point_settings();
    const Point& read_point_settings() const;
    static const PointLimits& read_point_limits();

    // Path settings
    Path* get_path_settings();
    const Path& read_path_settings() const;
    static const PathLimits& read_path_limits();

    // Surface settings
    Surface* get_surface_settings();
    const Surface& read_surface_settings() const;
    static const SurfaceLimits& read_surface_limits();

private:
    mutable std::unique_ptr<General> general_settings;
    mutable std::unique_ptr<Point> point_settings;
    mutable std::unique_ptr<Path> path_settings;
    mutable std::unique_ptr<Surface> surface_settings;
};
