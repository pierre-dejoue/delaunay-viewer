// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include "draw_command.h"
#include "drawing_options.h"

#include <base/canvas.h>
#include <imgui/imgui.h>

template <typename F>
void update_imgui_draw_list(ImDrawList& draw_list, const DrawCommands<F>& draw_commands, const Canvas<F>& canvas, const DrawingOptions& options);
