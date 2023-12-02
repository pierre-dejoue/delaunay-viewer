#pragma once

#if defined(__GNUC__)
#pragma GCC system_header
#endif

#include <portable-file-dialogs.h>

#include <filesystem>
#include <vector>

namespace pfd
{
    std::vector<std::filesystem::path> source_paths(open_file&& widget);
    std::filesystem::path target_path(save_file&& widget);
    std::filesystem::path folder_path(select_folder&& widget);
}

// Prevent namespace pollution
#ifdef _WIN32
#undef min
#undef max
#endif
