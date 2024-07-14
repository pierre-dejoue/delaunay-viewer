// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <base/color_data.h>
#include <shapes/shapes.h>

struct PrimitiveProperties
{
    PrimitiveProperties(ColorData color = { 0.f, 0.f, 0.f, 1.f }, bool draw = true) : color(color), draw(draw) {}
    ColorData color;
    bool draw;
};

template <typename F>
struct DrawCommand
{
    DrawCommand(const shapes::AllShapes<F>& shape);
    const shapes::AllShapes<F>* shape;
    PrimitiveProperties vertices;
    PrimitiveProperties edges;
    PrimitiveProperties faces;
};

template <typename F>
using DrawCommands = std::vector<DrawCommand<F>>;

template <typename F>
DrawCommand<F>::DrawCommand(const shapes::AllShapes<F>& shape)
    : shape(&shape)
    , vertices()
    , edges()
    , faces()
{ }
