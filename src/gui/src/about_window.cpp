// Copyright (c) 2025 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include "about_window.h"

#include "project.h"

#include <imgui/imgui.h>
#include <stdutils/macros.h>

#include <sstream>
#include <string>

namespace {

std::string s_about_window_title()
{
    std::stringstream out;
    out << "About " << project::get_name();
    return out.str();
}

std::string s_application_copyright()
{
    std::stringstream out;
    out << project::get_short_copyright() << "\n\n";
    out << project::get_short_license();
    return out.str();
}

} // namespace

bool AboutWindow::visit()
{
    static const std::string window_title = s_about_window_title();
    static const std::string application_copyright = s_application_copyright();

    bool keep_open = true;
    constexpr ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoCollapse
                                         | ImGuiWindowFlags_NoResize
                                         | ImGuiWindowFlags_NoSavedSettings;

    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    IGNORE_RETURN ImGui::Begin(window_title.c_str(), &keep_open, win_flags);
    {
        ImGui::BeginTable("about", 3, ImGuiTableFlags_None);
        {
            ImGui::TableSetupColumn("pre", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("text", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("post", ImGuiTableColumnFlags_WidthFixed);
            const float pre_column_width = 30.f;
            const float post_column_width = 120.f;
            const float dummy_row_min_height = 20.f;
            const float app_name_row_min_height = 40.f;

            // Row 0 (Blank, used for formatting)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None);
                ImGui::TableSetColumnIndex(0);
                ImGui::Dummy(ImVec2(pre_column_width,  dummy_row_min_height));
                ImGui::TableSetColumnIndex(2);
                ImGui::Dummy(ImVec2(post_column_width, dummy_row_min_height));
            }
            // Row 1 (App name)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, app_name_row_min_height);
                ImGui::TableSetColumnIndex(1);
                ImGui::TextLinkOpenURL(project::get_name().data(), project::get_website().data());
                ImGui::SameLine();
                ImGui::TextUnformatted(project::get_version_string().data());
            }
            // Row 2 (Copyright notice)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(application_copyright.c_str());
            }
            // Row 3 (Blank)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(1);
                ImGui::Dummy(ImVec2(1.f, dummy_row_min_height));
            }
            // Row 4 (OK button)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button("OK")) { keep_open = false; }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
    return keep_open;
}
