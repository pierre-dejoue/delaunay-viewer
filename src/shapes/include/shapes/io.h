// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
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
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace shapes {
namespace io {

template <typename F>
struct ShapeWrapper
{
    ShapeWrapper(shapes::AllShapes<F>&& shape, std::string descr = "") : shape(std::move(shape)), descr(descr) {}

    // TODO: Fix properly. This ctor might move the geometry if T is resolved into a reference type
    //template <typename T>
    //ShapeWrapper(T&& geom, std::string descr = "") : shape(std::move(geom)), descr(descr) {}

    template <typename T>
    ShapeWrapper(const T& geom, std::string descr = "") : shape(geom), descr(descr) {}

    shapes::AllShapes<F> shape;
    std::string descr;
};

template <typename F>
using ShapeAggregate = std::vector<ShapeWrapper<F>>;

//
// DAT format
//
namespace dat {
    ShapeAggregate<double> parse_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept;
    ShapeAggregate<double> parse_shapes_from_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;

    void save_shapes_as_stream(std::ostream& outputstream, const ShapeAggregate<double>& shapes, const stdutils::io::ErrorHandler& err_handler) noexcept;
    void save_shapes_as_file(std::filesystem::path filepath, const ShapeAggregate<double>& shapes, const stdutils::io::ErrorHandler& err_handler, std::string_view head_comment = "") noexcept;

    // Use for logs, trace files...
    void save_shapes_as_oneliner_stream(std::ostream& outputstream, const ShapeAggregate<double>& shapes, std::string_view postfix = "") noexcept;
}

//
// CDT format
//
namespace cdt {
    unsigned int peek_point_dimension(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept;
    unsigned int peek_point_dimension(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;
    shapes::Soup2d<double> parse_2d_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept;
    shapes::Soup2d<double> parse_2d_shapes_from_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;
    shapes::Soup3d<double> parse_3d_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept;
    shapes::Soup3d<double> parse_3d_shapes_from_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept;
}

} // namespace io
} // namespace shapes
