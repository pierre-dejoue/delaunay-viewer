// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "drawing_settings.h"

DrawingOptions drawing_options_from_settings(const Settings& settings)
{
    DrawingOptions options;

    const auto& point_settings      = settings.read_point_settings();
    const auto& path_settings       = settings.read_path_settings();
    const auto& surface_settings    = settings.read_surface_settings();

    options.point_options.show      = point_settings.show;
    options.point_options.size      = point_settings.size;
    options.path_options.show       = path_settings.show;
    options.path_options.width      = path_settings.width;
    options.surface_options.alpha   = surface_settings.alpha;
    options.surface_options.show    = surface_settings.show;

    return options;
}
