#pragma once

#include <shapes/path.h>
#include <shapes/point_cloud.h>
#include <shapes/shapes.h>
#include <shapes/soup.h>
#include <stdutils/io.h>

#include <cstdint>
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

//
// DAT format
//
namespace dat
{
    ShapeAggregate<double> parse_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept;
    ShapeAggregate<double> parse_shapes_from_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;
}

//
// CDT format
//
namespace cdt
{
    int peek_point_dimension(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;
    shapes::Soup2d<double> parse_2d_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept;
    shapes::Soup2d<double> parse_2d_shapes_from_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;
    shapes::Soup3d<double> parse_3d_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept;
    shapes::Soup3d<double> parse_3d_shapes_from_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;
}


} // namespace io
} // namespace shapes
