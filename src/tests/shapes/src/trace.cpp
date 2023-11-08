// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <trace.h>

#include <filesystem>

std::filesystem::path stdutils::trace_folder_path()
{
    return STDUTILS_TRACE_FILE_FOLDER;
}
