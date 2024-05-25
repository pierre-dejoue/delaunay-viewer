// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include "renderer.h"
#include "draw_command.h"
#include "drawing_options.h"

template <typename F>
void update_opengl_draw_list(renderer::DrawList& draw_list, const DrawCommands<F>& draw_commands, bool update_buffers, const DrawingOptions& options);
