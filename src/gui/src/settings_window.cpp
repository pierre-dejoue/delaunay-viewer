#include "settings_window.h"

#include "settings.h"

#include "imgui_helpers.h"
#include <imgui_wrap.h>

#include <cassert>


SettingsWindow::SettingsWindow(Settings& settings)
    : m_settings(settings)
    , m_title("Settings")
{
}

void SettingsWindow::visit(bool& can_be_erased, ScreenPos& initial_pos)
{
    can_be_erased = false;        // Cannot close

    const float max_width = 350.f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(200.f, 200.f), ImVec2(max_width, FLT_MAX));
    ImGui::SetNextWindowPos(to_imgui_vec2(initial_pos), ImGuiCond_Once);
    constexpr ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    if (!ImGui::Begin(m_title.c_str(), nullptr, win_flags))
    {
        // Collapsed
        ImGui::End();
        return;
    }
    initial_pos.x = ImGui::GetWindowPos().x + max_width + 1.f;

    const ImVec2 spacing = { 10.f, 10.f };
    Settings::General* general_settings = m_settings.get_general_settings();
    if (general_settings)
    {
        //const auto& limits = m_settings.read_general_limits();
        //ImGui::Dummy(spacing); First section, so no spacing required
        ImGui::BulletText("General");
        ImGui::Indent();
        ImGui::Checkbox("Flip Y-axis", &(general_settings->flip_y));
        ImGui::Checkbox("Imgui Renderer", &(general_settings->imgui_renderer));
        ImGui::Unindent();
    }

    Settings::Point* point_settings = m_settings.get_point_settings();
    if (point_settings)
    {
        const auto& limits = m_settings.read_point_limits();

        ImGui::Dummy(spacing);
        ImGui::BulletText("Points");
        ImGui::Indent();
        ImGui::Checkbox("Show##Point", &(point_settings->show));
        ImGui::SliderFloat("Size ratio##Point", &point_settings->size, limits.size.min, limits.size.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Unindent();
    }

    Settings::Path* path_settings = m_settings.get_path_settings();
    if (path_settings)
    {
        const auto& limits = m_settings.read_path_limits();
        ImGui::Dummy(spacing);
        ImGui::BulletText("Curve Segments");
        ImGui::Indent();
        ImGui::Checkbox("Show##Path", &(path_settings->show));
        ImGui::SliderFloat("Width##Path", &path_settings->width, limits.width.min, limits.width.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Unindent();
    }

    Settings::Surface* surface_settings = m_settings.get_surface_settings();
    if (surface_settings)
    {
        const auto& limits = m_settings.read_surface_limits();
        ImGui::Dummy(spacing);
        ImGui::BulletText("Faces");
        ImGui::Indent();
        ImGui::Checkbox("Show##Surface", &(surface_settings->show));
        ImGui::SliderFloat("Alpha##Surface", &surface_settings->alpha, limits.alpha.min, limits.alpha.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Unindent();
    }

    ImGui::End();
}
