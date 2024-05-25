// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include "draw_command.h"

struct DrawingOptions
{
    struct Point
    {
        bool show;
        float size;
    };
    struct Path
    {
        bool show;
        float width;
    };
    struct Surface
    {
        bool show;
        float alpha;
    };

    Point point_options{};
    Path path_options{};
    Surface surface_options{};
    PrimitiveProperties vertices{};
    PrimitiveProperties edges{};
    PrimitiveProperties faces{};
};
