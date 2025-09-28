// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <array>

void imgui_set_style(bool dark_mode);

std::array<float, 4> get_window_background_color(bool dark_mode);
