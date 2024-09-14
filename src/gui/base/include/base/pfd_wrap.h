// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#if defined(__GNUC__)
#pragma GCC system_header
#endif

#include <portable-file-dialogs.h>

#include <filesystem>
#include <vector>

// Add a few helper functions in the library's namespace
namespace pfd {

std::vector<std::filesystem::path> source_paths(open_file&& widget);
std::filesystem::path target_path(save_file&& widget);
std::filesystem::path folder_path(select_folder&& widget);

} // namespace pfd

// Prevent namespace pollution
#ifdef _WIN32
#undef min
#undef max
#endif
