// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include "renderer.h"
#include "draw_command.h"
#include "drawing_options.h"

#include <gui/abstract/canvas.h>

#include <memory>

template <typename F>
void update_opengl_draw_list(renderer::DrawList& draw_list, const DrawCommands<F>& draw_commands, bool update_buffers, const DrawingOptions& options);

template <typename F>
class CBPSegmentation {
public:
    CBPSegmentation();
    ~CBPSegmentation();

    void clear_all();

    const DrawCommands<F>& convert_cbps(const DrawCommands<F>& draw_commands, const Canvas<float>& viewport_canvas, bool geometry_has_changed, bool& new_cbp_segmentation);

private:
    struct Impl;
    std::unique_ptr<Impl> p_impl;
};
