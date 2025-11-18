// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <gui/abstract/color_data.h>
#include <shapes/shapes.h>

struct PrimitiveProperties
{
    PrimitiveProperties(ColorData color = COLOR_DATA_BLACK, bool draw = true) : color(color), draw(draw) {}
    ColorData color;
    bool draw;
};

template <typename F>
struct DrawCommand
{
    using ShapePtr = const shapes::AllShapes<F>*;

    explicit DrawCommand(const shapes::AllShapes<F>& shape);

    ShapePtr            shape_ptr;
    PrimitiveProperties vertices;
    PrimitiveProperties edges;
    PrimitiveProperties faces;
};

template <typename F>
using DrawCommands = std::vector<DrawCommand<F>>;

template <typename F>
DrawCommand<F>::DrawCommand(const shapes::AllShapes<F>& shape)
    : shape_ptr(&shape)
    , vertices()
    , edges()
    , faces()
{ }
