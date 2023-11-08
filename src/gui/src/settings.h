#pragma once

#include "parameter.h"

#include <memory>

class Settings
{
public:
    struct GeneralLimits
    {
        Parameter::Limits<bool> flip_y;
        Parameter::Limits<bool> imgui_renderer;
        Parameter::Limits<bool> cdt;
        Parameter::Limits<bool> proximity_graphs;
    };
    struct General
    {
        bool flip_y;        // If false, the Y-axis is in the "up" direction.
        bool imgui_renderer;
        bool cdt;
        bool proximity_graphs;
    };
    struct PointLimits
    {
        Parameter::Limits<bool> show;
        Parameter::Limits<float> size;
    };
    struct Point
    {
        bool show;
        float size;
    };
    struct PathLimits
    {
        Parameter::Limits<bool> show;
        Parameter::Limits<float> width;
    };
    struct Path
    {
        bool show;
        float width;
    };
    struct SurfaceLimits
    {
        Parameter::Limits<bool> show;
        Parameter::Limits<float> alpha;
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
