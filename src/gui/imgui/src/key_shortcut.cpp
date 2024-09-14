// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <imgui/key_shortcut.h>

namespace ImGui {
namespace key_shortcut {

const KeyShortcut& open()
{
    constexpr ImGuiKeyChord KEY_CHORD = ImGuiMod_Ctrl | ImGuiKey_O;
#if defined(__APPLE__)
    static KeyShortcut shortcut(KEY_CHORD, "Cmd+O");
#else
    static KeyShortcut shortcut(KEY_CHORD, "Ctrl+O");
#endif
    return shortcut;
}

const KeyShortcut& quit()
{
#if defined(__APPLE__)
    static KeyShortcut shortcut(ImGuiMod_Ctrl | ImGuiKey_Q, "Cmd+Q");
#else
    static KeyShortcut shortcut(ImGuiMod_Alt | ImGuiKey_F4, "Alt+F4");
#endif
    return shortcut;
}

} // namespace key_shortcut
} // namespace ImGui
