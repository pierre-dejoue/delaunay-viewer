// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <base/pfd_wrap.h>

#include <algorithm>

namespace pfd {

namespace {

// Portable File Dialogs encodes all paths as UTF8 strings
std::filesystem::path to_fs_path(const std::string& pfd_path)
{
    // TODO: In C++20 u8path gets deprecated, instead use std::u8string as argument to the path ctor
    return std::filesystem::u8path(pfd_path);
}

} // namespace

std::vector<std::filesystem::path> source_paths(open_file&& widget)
{
    std::vector<std::filesystem::path> result;
    const auto pfd_result = widget.result();
    result.reserve(pfd_result.size());
    std::transform(std::cbegin(pfd_result), std::cend(pfd_result), std::back_inserter(result), [](const std::string& path) { return to_fs_path(path); });
    return result;
}

std::filesystem::path target_path(save_file&& widget)
{
    return to_fs_path(widget.result());
}

std::filesystem::path folder_path(select_folder&& widget)
{
    return to_fs_path(widget.result());
}

} // namespace pfd
