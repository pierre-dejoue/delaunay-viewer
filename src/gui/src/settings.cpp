#include "settings.h"

#include <cassert>
#include <limits>

namespace
{

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

} // Anonymous namespace

Settings::Settings()
    : point_settings()
    , path_settings()
    , surface_settings()
{
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

void Settings::open_window()
{
    get_settings_window();

    read_point_settings();
    read_surface_settings();
    read_surface_settings();
}

void Settings::visit_window(bool& can_be_erased)
{
    can_be_erased = false;
    if (point_settings || path_settings || surface_settings || settings_window)
    {
        auto& settings_window_ref = get_settings_window();
        bool window_can_be_erased = false;
        settings_window_ref.visit(window_can_be_erased);
        assert(!window_can_be_erased); // Do not erase settings window once open
        can_be_erased &= window_can_be_erased;
    }
}

SettingsWindow& Settings::get_settings_window()
{
    if (!settings_window)
    {
        settings_window = std::make_unique<SettingsWindow>(*this);
    }
    assert(settings_window);
    return *settings_window;
}