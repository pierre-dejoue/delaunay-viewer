#include "settings_window.h"

#include "settings.h"

#include <imgui/imgui.h>

#include <cassert>


SettingsWindow::SettingsWindow(Settings& settings, DtTracker<scalar>& dt_tracker)
    : m_settings(settings)
    , m_dt_tracker(dt_tracker)
    , m_title("Settings")
{ }

void SettingsWindow::visit(bool& can_be_erased, const WindowLayout& win_pos_sz)
{
    can_be_erased = false;        // Cannot close

    ImGui::SetNextWindowPosAndSize(win_pos_sz);
    constexpr ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoCollapse
                                         | ImGuiWindowFlags_NoMove
                                         | ImGuiWindowFlags_NoResize
                                         | ImGuiWindowFlags_NoSavedSettings;

    if (!ImGui::Begin(m_title.c_str(), nullptr, win_flags))
    {
        // Collapsed
        ImGui::End();
        return;
    }

    const ImVec2 spacing = { 10.f, 10.f };
    Settings::General* general_settings = m_settings.get_general_settings();
    if (general_settings)
    {
        //const auto& limits = m_settings.read_general_limits();
        //ImGui::Dummy(spacing); First section, so no spacing required
        ImGui::BulletTextUnformatted("General");
        ImGui::Indent();
        ImGui::Checkbox("Flip Y-axis", &(general_settings->flip_y));
        ImGui::Checkbox("Line smooth", &(general_settings->line_smooth));
        ImGui::Checkbox("Constrained Delaunay", &(general_settings->cdt));
        ImGui::Checkbox("Proximity Graphs", &(general_settings->proximity_graphs));
        ImGui::Unindent();
    }

    {
        ImGui::Dummy(spacing);
        ImGui::BulletTextUnformatted("Triangulation algos");
        ImGui::Indent();
        for (auto& algo : m_dt_tracker.list_algos())
        {
            ImGui::Checkbox(algo.impl.name.c_str(), &(algo.active));
        }
        ImGui::Unindent();
    }

    Settings::Point* point_settings = m_settings.get_point_settings();
    if (point_settings)
    {
        const auto& limits = m_settings.read_point_limits();

        ImGui::Dummy(spacing);
        ImGui::BulletTextUnformatted("Points");
        ImGui::Indent();
        ImGui::Checkbox("Show##Point", &(point_settings->show));
        ImGui::SameLine();
        ImGui::SliderFloat("Size##Point", &point_settings->size, limits.size.min, limits.size.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Unindent();
    }

    Settings::Path* path_settings = m_settings.get_path_settings();
    if (path_settings)
    {
        ImGui::Dummy(spacing);
        ImGui::BulletTextUnformatted("Lines");
        ImGui::Indent();
        ImGui::Checkbox("Show##Path", &(path_settings->show));
        ImGui::Unindent();
    }

    Settings::Surface* surface_settings = m_settings.get_surface_settings();
    if (surface_settings)
    {
        const auto& limits = m_settings.read_surface_limits();
        ImGui::Dummy(spacing);
        ImGui::BulletTextUnformatted("Faces");
        ImGui::Indent();
        ImGui::Checkbox("Show##Surface", &(surface_settings->show));
        ImGui::SameLine();
        ImGui::SliderFloat("Alpha##Surface", &surface_settings->alpha, limits.alpha.min, limits.alpha.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Unindent();
    }

    ImGui::End();
}
