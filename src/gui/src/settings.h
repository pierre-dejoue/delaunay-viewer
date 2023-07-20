#pragma once

#include "parameter.h"
#include "settings_window.h"

#include <memory>

class Settings
{
public:
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

    void open_window();
    void visit_window(bool& can_be_erased);

private:
    SettingsWindow& get_settings_window();

private:
    mutable std::unique_ptr<Point> point_settings;
    mutable std::unique_ptr<Path> path_settings;
    mutable std::unique_ptr<Surface> surface_settings;
    std::unique_ptr<SettingsWindow> settings_window;
};
