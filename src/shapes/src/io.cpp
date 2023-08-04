#include <shapes/io.h>

#include <shapes/traits.h>
#include <stdutils/string.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iterator>
#include <istream>
#include <sstream>
#include <vector>


namespace shapes
{
namespace io
{

namespace
{

enum class ShapeType
{
    POINT_CLOUD,
    POINT_PATH,
    CUBIC_BEZIER_PATH
};

template <typename F>
struct ShapeBuffer
{
    using Vertex = std::array<F, 3>;
    std::vector<Vertex> vertices;
    unsigned int dim = 0;
    bool closed = true;
    ShapeType type = ShapeType::POINT_CLOUD;
};

template <typename F>
bool parse_coordinate_line(std::istringstream line_stream, ShapeBuffer<F>& buffer)
{
    std::istream_iterator<F> fp_stream(line_stream);
    const std::istream_iterator<F> stream_end;
    typename ShapeBuffer<F>::Vertex vertex;
    vertex.fill(F{0});
    unsigned int idx = 0;
    while(fp_stream != stream_end && idx < 3)
        vertex[idx++] = *fp_stream++;
    if (idx > 0)
    {
        assert(std::all_of(std::cbegin(vertex), std::cend(vertex), [](const F& f) { return std::isfinite(f); }));
        buffer.vertices.push_back(vertex);
        buffer.dim = std::max(buffer.dim, idx);
        return true;
    }
    return false;
}

template <typename F>
void copy_shape_vertices(std::vector<Point2d<F>>& target, const ShapeBuffer<F>& source)
{
    for (const auto& arr: source.vertices)
        target.emplace_back(arr[0], arr[1]);
}

template <typename F>
void copy_shape_vertices(std::vector<Point3d<F>>& target, const ShapeBuffer<F>& source)
{
    for (const auto& arr: source.vertices)
        target.emplace_back(arr[0], arr[1], arr[2]);
}

template <typename F, int Dim>
void append_new_shape(const ShapeBuffer<F>& buffer, ShapeAggregate<F>& shapes, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    assert(buffer.dim == Dim);
    switch (buffer.type)
    {
        case ShapeType::POINT_CLOUD:
        {
            shapes::PointCloud<typename shapes::Traits<F, Dim>::Point> pc;
            copy_shape_vertices(pc.vertices, buffer);
            shapes.emplace_back(std::move(pc));
            break;
        }
        case ShapeType::POINT_PATH:
        {
            shapes::PointPath<typename shapes::Traits<F, Dim>::Point> pp;
            pp.closed = buffer.closed;
            copy_shape_vertices(pp.vertices, buffer);
            shapes.emplace_back(std::move(pp));
            break;
        }
        case ShapeType::CUBIC_BEZIER_PATH:
        {
            shapes::CubicBezierPath<typename shapes::Traits<F, Dim>::Point> cbp;
            cbp.closed = buffer.closed;
            copy_shape_vertices(cbp.vertices, buffer);
            if (shapes::valid_size(cbp))
                shapes.emplace_back(std::move(cbp));
            else
                err_handler(stdutils::io::Severity::WARNING, "Ignored a cubic bezier path with invalid size");
            break;
        }
    }
}

template <typename F>
ShapeAggregate<F> parse_shapes_dat_stream_gen(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    ShapeAggregate<F> result;
    constexpr std::size_t N = 512;
    std::array<char, N> line_buf;
    try
    {
        ShapeBuffer<F> buffer;
        while (inputstream.good())
        {
            inputstream.getline(&line_buf[0], N);
            if (!parse_coordinate_line<F>(std::istringstream(&line_buf[0]), buffer) || !inputstream.good())
            {
                std::istringstream line_stream(&line_buf[0]);
                std::istream_iterator<std::string> str_stream(line_stream);
                const std::istream_iterator<std::string> stream_end;
                const std::string first_token = ((str_stream != stream_end) ? *str_stream++ : "");
                const bool not_a_comment = !first_token.empty() && first_token[0] != '#';
                if (not_a_comment || inputstream.eof())
                {
                    // 1. Store the previous shape
                    if (!buffer.vertices.empty())
                    {
                        assert(buffer.dim > 0);
                        if (buffer.dim == 3)
                            append_new_shape<F, 3>(buffer, result, err_handler);
                        else
                            append_new_shape<F, 2>(buffer, result, err_handler);
                    }
                    // 2. Reset shape buffer
                    buffer = ShapeBuffer<F>();
                    // 3. Type of the next shape
                    const auto type_str = stdutils::string::tolower(first_token);
                    if (type_str == "point_path") { buffer.type = ShapeType::POINT_PATH; }
                    else if (type_str == "cubic_bezier_path") { buffer.type = ShapeType::CUBIC_BEZIER_PATH; }
                    else { buffer.type = ShapeType::POINT_CLOUD; }
                    // 4. Topology of the next shape
                    const std::string second_token = ((str_stream != stream_end) ? *str_stream++ : "");
                    const auto topo_str = stdutils::string::tolower(second_token);
                    buffer.closed = (topo_str != "open");
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        std::stringstream oss;
        oss << "Exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, oss.str());
    }

    return result;
}

template <typename F>
ShapeAggregate<F> parse_shapes_dat_file_gen(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    try
    {
        std::ifstream inputstream(filepath);
         if (inputstream.is_open())
        {
            return parse_shapes_dat_stream_gen<F>(inputstream, err_handler);
        }
        else
        {
            std::stringstream oss;
            oss << "Cannot open file " << filepath;
            err_handler(stdutils::io::Severity::FATAL, oss.str());
        }
    }
    catch(const std::exception& e)
    {
        std::stringstream oss;
        oss << "Exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, oss.str());
    }
    return ShapeAggregate<F>();
}

} // Anonymous namespace

ShapeAggregate<double> parse_shapes_dat_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    return parse_shapes_dat_stream_gen<double>(inputstream, err_handler);
}

ShapeAggregate<double> parse_shapes_dat_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    return parse_shapes_dat_file_gen<double>(filepath, err_handler);
}

} // namespace io
} // namespace shapes
