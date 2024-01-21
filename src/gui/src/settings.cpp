#include "settings.h"

#include <cassert>
#include <limits>

namespace {

    Settings::GeneralLimits general_settings_limits()
    {
        Settings::GeneralLimits result;

        result.flip_y = Parameter::limits_true;
        result.imgui_renderer = Parameter::limits_false;
        result.cdt = Parameter::limits_true;
        result.proximity_graphs = Parameter::limits_false;

        return result;
    }

    Settings::PointLimits point_settings_limits()
    {
        Settings::PointLimits result;

        result.show = Parameter::limits_true;

        result.size.def = 4.f;
        result.size.min = 1.f;
        result.size.max = 32.f;

        return result;
    }

    Settings::PathLimits path_settings_limits()
    {
        Settings::PathLimits result;

        result.show = Parameter::limits_true;

        result.width.def = 1.f;
        result.width.min = 1.f;
        result.width.max = 32.f;

        return result;
    }

    Settings::SurfaceLimits surface_settings_limits()
    {
        Settings::SurfaceLimits result;

        result.show = Parameter::limits_true;

        result.alpha.def = 0.75f;
        result.alpha.min = 0.0f;
        result.alpha.max = 1.f;

        return result;
    }

} // namespace

Settings::Settings()
{
    read_general_settings();
    read_point_settings();
    read_surface_settings();
    read_surface_settings();
}

Settings::General* Settings::get_general_settings()
{
    return general_settings.get();
}

const Settings::General& Settings::read_general_settings() const
{
    // Create if not existing
    if (!general_settings)
    {
        general_settings = std::make_unique<General>();
        general_settings->flip_y = read_general_limits().flip_y.def;
        general_settings->imgui_renderer = read_general_limits().imgui_renderer.def;
        general_settings->cdt = read_general_limits().cdt.def;
        general_settings->proximity_graphs = read_general_limits().proximity_graphs.def;
    }
    assert(general_settings);
    return *general_settings;
}

const Settings::GeneralLimits& Settings::read_general_limits()
{
    static Settings::GeneralLimits result = general_settings_limits();
    return result;
}

Settings::Point* Settings::get_point_settings()
{
    return point_settings.get();
}

const Settings::Point& Settings::read_point_settings() const
{
    // Create if not existing
    if (!point_settings)
    {
        point_settings = std::make_unique<Point>();
        point_settings->show = read_point_limits().show.def;
        point_settings->size = read_point_limits().size.def;
    }
    assert(point_settings);
    return *point_settings;
}

const Settings::PointLimits& Settings::read_point_limits()
{
    static Settings::PointLimits result = point_settings_limits();
    return result;
}

Settings::Path* Settings::get_path_settings()
{
    return path_settings.get();
}

const Settings::Path& Settings::read_path_settings() const
{
    // Create if not existing
    if (!path_settings)
    {
        path_settings = std::make_unique<Path>();
        path_settings->show = read_path_limits().show.def;
        path_settings->width = read_path_limits().width.def;
    }
    assert(path_settings);
    return *path_settings;
}

const Settings::PathLimits& Settings::read_path_limits()
{
    static Settings::PathLimits result = path_settings_limits();
    return result;
}

Settings::Surface* Settings::get_surface_settings()
{
    return surface_settings.get();
}

const Settings::Surface& Settings::read_surface_settings() const
{
    // Create if not existing
    if (!surface_settings)
    {
        surface_settings = std::make_unique<Surface>();
        surface_settings->show = read_surface_limits().show.def;
        surface_settings->alpha = read_surface_limits().alpha.def;
    }
    assert(surface_settings);
    return *surface_settings;
}

const Settings::SurfaceLimits& Settings::read_surface_limits()
{
    static Settings::SurfaceLimits result = surface_settings_limits();
    return result;
}
