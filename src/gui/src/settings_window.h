#pragma once

#include "window_layout.h"

#include <string>

class Settings;

class SettingsWindow
{
public:
    explicit SettingsWindow(Settings& settings);
    SettingsWindow(const SettingsWindow&) = delete;
    SettingsWindow& operator=(const SettingsWindow&) = delete;

    void visit(bool& can_be_erased, const WindowLayout& win_pos_sz);

private:
    Settings& m_settings;
    std::string m_title;
};
