#pragma once

#include <shapes/path.h>
#include <stdutils/io.h>

#include <filesystem>
#include <vector>


namespace svg
{

template <typename F>
struct Paths
{
    std::vector<shapes::PointPath2d<F>>        point_paths;
    std::vector<shapes::CubicBezierPath2d<F>>  cubic_bezier_paths;
};

namespace io
{

Paths<double> parse_svg_paths(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;

} // namespace io
} // namespace svg
