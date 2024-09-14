// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <imgui/imgui.h>

namespace ImGui {

/**
 * A keyboard shortcut + a platform dependent label
 */
struct KeyShortcut
{
    KeyShortcut(ImGuiKeyChord kc, const char* lbl = nullptr)
        : key_chord(kc)
        , label(lbl)
    { }

    ImGuiKeyChord key_chord;
    const char* label;
};

/**
 * A list of predefined shortcuts
 */
namespace key_shortcut {

const KeyShortcut& open();
const KeyShortcut& quit();

} // namespace key_shortcut
} // namespace ImGui
