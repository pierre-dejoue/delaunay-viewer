// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/io.h>
#include <svg/svg_paths.h>

#include <filesystem>

namespace svg {
namespace io {

Paths<double> parse_paths(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;

} // namespace io
} // namespace svg
