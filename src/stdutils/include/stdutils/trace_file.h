// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

// Trace files are primarily intended for UTests.
// Use them to output trace information in files managed by git.
// The idea is to follow the evolution of the trace output with that of the code in the git history.
namespace stdutils {

// Define in your test suite a function with the following prototype (in namespace stdutils!):
extern std::filesystem::path trace_folder_path();

// Might throw. Let it crash the unit test executable in that case.
template <typename CharT = char>
std::ofstream trace_open_file(std::string_view filename, std::filesystem::path subpath = std::filesystem::path(), bool silent = false)
{
    const std::filesystem::path trace_subfolder = trace_folder_path() / subpath;
    const std::filesystem::path trace_file = trace_subfolder / filename;
    if (!silent) { std::cout << "Open stdutils trace file: " << trace_file << std::endl; }

    // As per the documentation, if the directory already exist, does nothing
    std::filesystem::create_directories(trace_subfolder);

    return std::basic_ofstream<CharT>(trace_file);
}

} // namespace stdutils
