#pragma once

#include "imgui_wrap.h"

#include <string>

class Settings;

class SettingsWindow
{
public:
    explicit SettingsWindow(Settings& settings);
    SettingsWindow(const SettingsWindow&) = delete;
    SettingsWindow& operator=(const SettingsWindow&) = delete;

    void visit(bool& can_be_erased, ImVec2& initial_pos);

private:
    Settings& m_settings;
    std::string m_title;
};
