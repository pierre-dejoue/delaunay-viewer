// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <gui/abstract/color_data.h>

namespace gui_style {

ColorData get_window_background_color(bool dark_mode);

} // namespace gui_style

void imgui_set_style(bool dark_mode);
