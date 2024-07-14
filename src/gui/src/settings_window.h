#pragma once

#include "dt_tracker.h"
#include "viewport_window.h"

#include <base/window_layout.h>

#include <string>

class Settings;

class SettingsWindow
{
public:
    using scalar = ViewportWindow::scalar;

    SettingsWindow(Settings& settings, DtTracker<scalar>& dt_tracker);
    SettingsWindow(const SettingsWindow&) = delete;
    SettingsWindow& operator=(const SettingsWindow&) = delete;

    void visit(bool& can_be_erased, const WindowLayout& win_pos_sz);

private:
    Settings& m_settings;
    DtTracker<scalar>& m_dt_tracker;
    std::string m_title;
};
