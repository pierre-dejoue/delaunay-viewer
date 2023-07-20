#pragma once

#include <shapes/path.h>
#include <shapes/point_cloud.h>
#include <shapes/shapes.h>
#include <stdutils/io.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <variant>
#include <vector>


namespace shapes
{
namespace io
{

template <typename F>
using ShapeAggregate = std::vector<shapes::AllShapes<F>>;

ShapeAggregate<double> parse_shapes_dat_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept;
ShapeAggregate<double> parse_shapes_dat_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;

} // namespace io
} // namespace shapes
