// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <gui/base/pfd_wrap.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace pfd {

namespace {

// Portable File Dialogs encodes all paths as UTF8 strings
fs::path to_fs_path(const std::string& pfd_path)
{
    // TODO: In C++20 u8path gets deprecated, instead use std::u8string as argument to the path ctor
    return fs::u8path(pfd_path);
}

} // namespace

std::vector<fs::path> source_paths(open_file&& widget)
{
    std::vector<fs::path> result;
    const auto pfd_result = widget.result();
    result.reserve(pfd_result.size());
    std::transform(std::cbegin(pfd_result), std::cend(pfd_result), std::back_inserter(result), [](const std::string& path) { return to_fs_path(path); });
    return result;
}

fs::path target_path(save_file&& widget)
{
    return to_fs_path(widget.result());
}

fs::path folder_path(select_folder&& widget)
{
    return to_fs_path(widget.result());
}

} // namespace pfd
