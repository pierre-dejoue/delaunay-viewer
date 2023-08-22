#pragma once

#include <shapes/shapes.h>


template <typename F>
struct DrawCommand
{
    DrawCommand(const shapes::AllShapes<F>& shape);
    bool highlight;
    bool constraint_edges;
    const shapes::AllShapes<F>* shape;
};

template <typename F>
using DrawCommands = std::vector<DrawCommand<F>>;

template <typename F>
DrawCommand<F>::DrawCommand(const shapes::AllShapes<F>& shape)
    : highlight(false)
    , constraint_edges(false)
    , shape(&shape)
{ }
