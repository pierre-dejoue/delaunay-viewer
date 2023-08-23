#pragma once

#include "renderer.h"

#include <shapes/shapes.h>


template <typename F>
struct DrawCommand
{
    DrawCommand(const shapes::AllShapes<F>& shape);
    const shapes::AllShapes<F>* shape;
    renderer::ColorData point_color;
    renderer::ColorData edge_color;
    renderer::ColorData face_color;
};

template <typename F>
using DrawCommands = std::vector<DrawCommand<F>>;

template <typename F>
DrawCommand<F>::DrawCommand(const shapes::AllShapes<F>& shape)
    : shape(&shape)
    , point_color({ 0.f, 0.f, 0.f, 1.f })
    , edge_color({ 0.f, 0.f, 0.f, 1.f })
    , face_color({ 0.f, 0.f, 0.f, 1.f })
{ }
