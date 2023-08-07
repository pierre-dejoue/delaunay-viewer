#include <shapes/io.h>

#include <shapes/traits.h>
#include <stdutils/io.h>
#include <stdutils/string.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iterator>
#include <istream>
#include <numeric>
#include <set>
#include <sstream>
#include <type_traits>
#include <vector>


namespace shapes
{
namespace io
{
namespace
{

template <typename T, std::size_t MAX_DIM>
struct NumericLineBuffer
{
    static_assert(std::is_arithmetic_v<T>);
    using Entry = std::array<T, MAX_DIM>;
    std::vector<Entry> lines;
    unsigned int dim = 0;
};

template <typename T, std::size_t MAX_DIM>
bool parse_numeric_line(const std::string& line, NumericLineBuffer<T, MAX_DIM>& buffer, T default_val = T{0})
{
    std::istringstream line_stream(line);
    std::istream_iterator<T> numeric_stream(line_stream);
    const std::istream_iterator<T> stream_end;
    typename NumericLineBuffer<T, MAX_DIM>::Entry entry;
    entry.fill(default_val);
    unsigned int idx = 0;
    while(numeric_stream != stream_end && idx < MAX_DIM)
        entry[idx++] = *numeric_stream++;
    if (idx > 0)
    {
        buffer.lines.push_back(entry);
        stdutils::max_update(buffer.dim, idx);
        return true;
    }
    return false;
}

class TokenIterator
{
public:
    using str_iterator = std::istream_iterator<std::string>;
    TokenIterator(const std::string& line)
        : m_str_stream(line)
        , m_str_iterator(m_str_stream)
    {}

    std::string next_token()
    {
        static const str_iterator stream_end;
        return (m_str_iterator != stream_end) ? *m_str_iterator++ : "";
    }
private:
    std::istringstream  m_str_stream;
    str_iterator        m_str_iterator;
};

enum class ShapeType
{
    POINT_CLOUD,
    POINT_PATH,
    CUBIC_BEZIER_PATH
};

template <typename F, std::size_t MAX_DIM>
struct ShapeBuffer
{
    static_assert(std::is_floating_point_v<F>);
    NumericLineBuffer<F, MAX_DIM> vertices;
    bool closed = true;
    ShapeType type = ShapeType::POINT_CLOUD;
};

template <typename F, std::size_t MAX_DIM>
void append_vertices(std::vector<Point2d<F>>& target, const NumericLineBuffer<F, MAX_DIM>& source)
{
    static_assert(MAX_DIM >= 2);
    target.reserve(target.size() + source.lines.size());
    for (const auto& arr: source.lines) { target.emplace_back(arr[0], arr[1]); }
}

template <typename F, std::size_t MAX_DIM>
void append_vertices(std::vector<Point3d<F>>& target, const NumericLineBuffer<F, MAX_DIM>& source)
{
    static_assert(MAX_DIM >= 3);
    target.reserve(target.size() + source.lines.size());
    for (const auto& arr: source.lines) { target.emplace_back(arr[0], arr[1], arr[2]); }
}

} // Anonymous namespace

namespace dat
{
namespace
{

template <typename F, int DIM, int MAX_DIM>
void append_new_shape(const ShapeBuffer<F, MAX_DIM>& buffer, ShapeAggregate<F>& shapes, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    static_assert(DIM <= MAX_DIM);
    switch (buffer.type)
    {
        case ShapeType::POINT_CLOUD:
        {
            shapes::PointCloud<typename shapes::Traits<F, DIM>::Point> pc;
            append_vertices(pc.vertices, buffer.vertices);
            shapes.emplace_back(std::move(pc));
            break;
        }
        case ShapeType::POINT_PATH:
        {
            shapes::PointPath<typename shapes::Traits<F, DIM>::Point> pp;
            pp.closed = buffer.closed;
            append_vertices(pp.vertices, buffer.vertices);
            shapes.emplace_back(std::move(pp));
            break;
        }
        case ShapeType::CUBIC_BEZIER_PATH:
        {
            shapes::CubicBezierPath<typename shapes::Traits<F, DIM>::Point> cbp;
            cbp.closed = buffer.closed;
            append_vertices(cbp.vertices, buffer.vertices);
            if (shapes::valid_size(cbp))
                shapes.emplace_back(std::move(cbp));
            else
                err_handler(stdutils::io::Severity::WARN, "Ignored a cubic bezier path with invalid size");
            break;
        }
    }
}

template <typename F>
ShapeAggregate<F> parse_shapes_from_stream_gen(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler)
{
    ShapeAggregate<F> result;
    std::string line_buf;
    ShapeBuffer<F, 3> buffer;
    while (inputstream.good())
    {
        while (std::getline(inputstream, line_buf) && parse_numeric_line(line_buf, buffer.vertices)) {}
        TokenIterator token_iterator(line_buf);
        const std::string first_token = token_iterator.next_token();
        const bool not_a_comment = first_token.substr(0, 1) != "#";
        if (not_a_comment || inputstream.eof())
        {
            // 1. Store the previous shape
            if (!buffer.vertices.lines.empty())
            {
                assert(buffer.vertices.dim > 0);
                if (buffer.vertices.dim == 3)
                    append_new_shape<F, 3>(buffer, result, err_handler);
                else
                    append_new_shape<F, 2>(buffer, result, err_handler);
            }
            // 2. Reset shape buffer
            buffer = ShapeBuffer<F, 3>();
            // 3. Type of the next shape
            const auto type_str = stdutils::string::tolower(first_token);
            if (type_str == "point_path") { buffer.type = ShapeType::POINT_PATH; }
            else if (type_str == "cubic_bezier_path") { buffer.type = ShapeType::CUBIC_BEZIER_PATH; }
            else { buffer.type = ShapeType::POINT_CLOUD; }
            // 4. Topology of the next shape
            const auto topo_str = stdutils::string::tolower(token_iterator.next_token());
            buffer.closed = (topo_str != "open");
        }
    }
    return result;
}

} // Anonymous namespace

ShapeAggregate<double> parse_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    try
    {
        return parse_shapes_from_stream_gen<double>(inputstream, err_handler);
    }
    catch (const std::exception& e)
    {
        std::stringstream oss;
        oss << "Exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, oss.str());
    }
    return ShapeAggregate<double>();
}

ShapeAggregate<double> parse_shapes_from_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    return stdutils::io::parse_file_generic<ShapeAggregate<double>, char>(parse_shapes_from_stream_gen<double>, filepath, err_handler);
}
} // namespace dat

namespace cdt
{
namespace
{

bool check_expected_size(std::string_view id, std::size_t sz0, std::size_t sz1, const stdutils::io::ErrorHandler& err_handler, stdutils::io::SeverityCode sev)
{
    if (sz0 != sz1)
    {
        std::stringstream out;
        out << "Unexpected number of " << id << ": " << sz0 << " vs " << sz1;
        err_handler(sev, out.str());
        return false;
    }
    return true;
}

enum class CDT_State
{
    HeaderLine,
    ParseVertices,
    ParseEdgeIndices,
    ParseTriangleIndices,
    Done
};

template <typename P, typename I, int DIM>
shapes::Soup<P, I> parse_shapes_from_stream_gen(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler)
{
    using F = typename P::scalar;
    shapes::Soup<P, I> result;
    std::string line_buf;
    CDT_State cdt_state = CDT_State::HeaderLine;
    I nb_vertices = 0;
    I nb_edges = 0;
    I nb_triangles = 0;
    std::vector<typename shapes::Traits<F, DIM>::Point> vertices;
    graphs::EdgeSoup<I> edges;
    graphs::TriangleSoup<I> triangles;
    constexpr I undef = graphs::IndexTraits<I>::undef();
    std::size_t line_idx = 0;
    while (inputstream.good() && cdt_state != CDT_State::Done)
    {
        switch (cdt_state)
        {
            case CDT_State::HeaderLine:
            {
                NumericLineBuffer<I, 3> count_buffer;
                while (std::getline(inputstream, line_buf) && !parse_numeric_line(line_buf, count_buffer))
                {
                    TokenIterator token_iterator(line_buf);
                    const bool not_a_comment = token_iterator.next_token().substr(0, 1) != "#";
                    if (not_a_comment)
                    {
                        std::stringstream out;
                        out << "CDT_State: HeaderLine. Invalid line #" << line_idx;
                        err_handler(stdutils::io::Severity::ERR, out.str());
                    }
                }
                if (count_buffer.lines.size() == 1)
                {
                    nb_vertices = count_buffer.lines.front()[0];
                    nb_edges = count_buffer.lines.front()[1];
                    nb_triangles = count_buffer.lines.front()[2];
                    cdt_state = CDT_State::ParseVertices;
                }
                else
                {
                    err_handler(stdutils::io::Severity::ERR, "CDT_State: HeaderLine. Could not find the header line.");
                    cdt_state = CDT_State::Done;
                }
                line_idx++;
                break;
            }
            case CDT_State::ParseVertices:
            {
                NumericLineBuffer<F, DIM> vertex_buffer;
                while (vertex_buffer.lines.size() < nb_vertices && std::getline(inputstream, line_buf) && parse_numeric_line(line_buf, vertex_buffer)) { line_idx++; }
                append_vertices(vertices, vertex_buffer);
                cdt_state = CDT_State::ParseEdgeIndices;
                break;
            }
            case CDT_State::ParseEdgeIndices:
            {
                NumericLineBuffer<I, 2> edge_buffer;
                while (edge_buffer.lines.size() < nb_edges && std::getline(inputstream, line_buf) && parse_numeric_line(line_buf, edge_buffer, undef)) { line_idx++; }
                edges.reserve(nb_edges);
                for (const auto& arr : edge_buffer.lines) { edges.emplace_back(arr[0], arr[1]); }
                cdt_state = CDT_State::ParseTriangleIndices;
                break;
            }
            case CDT_State::ParseTriangleIndices:
            {
                NumericLineBuffer<I, 3> triangle_buffer;
                while (triangle_buffer.lines.size() < nb_triangles && std::getline(inputstream, line_buf) && parse_numeric_line(line_buf, triangle_buffer, undef)) { line_idx++; }
                triangles.reserve(nb_triangles);
                for (const auto& arr : triangle_buffer.lines) { triangles.emplace_back(arr[0], arr[1], arr[2]); }
                cdt_state = CDT_State::Done;
                break;
            }
            case CDT_State::Done:
            default:
                assert(0);
                break;
        }
    }
    // Check that we read as many vertices/edges/triangles as advertised by the header line
    if (!check_expected_size("vertices", vertices.size(), nb_vertices, err_handler, stdutils::io::Severity::ERR)) { return result; }
    if (!check_expected_size("edges", edges.size(), nb_edges, err_handler, stdutils::io::Severity::ERR)) { return result; }
    if (!check_expected_size("triangles", triangles.size(), nb_triangles, err_handler, stdutils::io::Severity::ERR)) { return result; }

    // Copy indices, eliminating invalid edges or triangles (with warnings)
    {
        result.edges.indices.reserve(edges.size());
        std::set<graphs::Edge<I>> ordered_edges;
        std::copy_if(edges.cbegin(), edges.cend(), std::back_inserter(result.edges.indices), [&err_handler, &ordered_edges, nb_vertices](const auto& e) {
            if (e.first == e.second) { err_handler(stdutils::io::Severity::WARN, "Eliminated a loop edge"); return false; }
            if (e.first >= nb_vertices || e.second >= nb_vertices) { err_handler(stdutils::io::Severity::WARN, "Eliminated an edge with out of bound indices"); return false; }
            if (!ordered_edges.insert(graphs::ordered_edge(e)).second) { err_handler(stdutils::io::Severity::WARN, "Eliminated a duplicated edge"); return false; }
            return true;
        });
        assert(graphs::is_valid(result.edges.indices));
    }
    {
        result.triangles.faces.reserve(triangles.size());
        std::copy_if(triangles.cbegin(), triangles.cend(), std::back_inserter(result.triangles.faces), [&err_handler, nb_vertices](const auto& t) {
            if (!graphs::is_valid(t)) { err_handler(stdutils::io::Severity::WARN, "Eliminated an invalid triangle"); return false; }
            if (t[0] >= nb_vertices || t[1] >= nb_vertices || t[2] >= nb_vertices) { err_handler(stdutils::io::Severity::WARN, "Eliminated a triangle with out of bound indices"); return false; }
            return true;
        });
        assert(graphs::is_valid(result.triangles.faces));
    }

    // Copy vertices
    result.edges.vertices.reserve(1 + nb_edges);            // Lower bound
    result.triangles.vertices.reserve(2 + nb_triangles);    // Lower bound
    std::vector<I> vertices_in_edges(nb_vertices + 1, I{0});            // Also used to remap the indices
    std::vector<I> vertices_in_triangles(nb_vertices + 1, I{0});        // Also used to remap the indices
    std::for_each(result.edges.indices.cbegin(), result.edges.indices.cend(), [&vertices_in_edges](const auto& e) {
        vertices_in_edges[e.first + 1] = 1;
        vertices_in_edges[e.second + 1] = 1; });
    std::for_each(result.triangles.faces.cbegin(), result.triangles.faces.cend(), [&vertices_in_triangles](const auto& t) {
        vertices_in_triangles[t[0] + 1] = 1;
        vertices_in_triangles[t[1] + 1] = 1;
        vertices_in_triangles[t[2] + 1] = 1; });
    assert(vertices.size() == static_cast<std::size_t>(nb_vertices));
    for (std::size_t idx = 0; idx < nb_vertices; idx++)
    {
        const auto& p = vertices[idx];
        const bool p_in_edges = vertices_in_edges[idx + 1];
        const bool p_in_triangles = vertices_in_triangles[idx + 1];
        if (!p_in_edges && !p_in_triangles)
            result.point_cloud.vertices.emplace_back(p);
        if (p_in_edges)
            result.edges.vertices.emplace_back(p);
        if (p_in_triangles)
            result.triangles.vertices.emplace_back(p);
    }

    // Finally, remap the indices of the edges and the triangles
    std::partial_sum(vertices_in_edges.cbegin(), vertices_in_edges.cend(), vertices_in_edges.begin());
    std::partial_sum(vertices_in_triangles.cbegin(), vertices_in_triangles.cend(), vertices_in_triangles.begin());
    for (auto& e : result.edges.indices)
    {
        e.first = vertices_in_edges[e.first];
        e.second = vertices_in_edges[e.second];
    }
    for (auto& t : result.triangles.faces)
    {
        t[0] = vertices_in_triangles[t[0]];
        t[1] = vertices_in_triangles[t[1]];
        t[2] = vertices_in_triangles[t[2]];
    }

    assert(shapes::is_valid(result.edges));
    assert(shapes::is_valid(result.triangles));
    return result;
}

}

int peek_point_dimension(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    // TODO implement!
    UNUSED(filepath);
    UNUSED(err_handler);
    return 2;
}

shapes::Soup2d<double> parse_2d_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    using P = shapes::Point2d<double>;
    using I = std::uint32_t;
    try
    {
        return parse_shapes_from_stream_gen<P, I, 2>(inputstream, err_handler);
    }
    catch (const std::exception& e)
    {
        std::stringstream oss;
        oss << "Exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, oss.str());
    }
    return shapes::Soup2d<double>();
}

shapes::Soup2d<double> parse_2d_shapes_from_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    using P = shapes::Point2d<double>;
    using I = std::uint32_t;
    return stdutils::io::parse_file_generic<shapes::Soup<P,I>, char>(parse_shapes_from_stream_gen<P, I, 2>, filepath, err_handler);}

shapes::Soup3d<double> parse_3d_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    using P = shapes::Point3d<double>;
    using I = std::uint32_t;
    try
    {
        return parse_shapes_from_stream_gen<P, I, 3>(inputstream, err_handler);
    }
    catch (const std::exception& e)
    {
        std::stringstream oss;
        oss << "Exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, oss.str());
    }
    return shapes::Soup3d<double>();
}

shapes::Soup3d<double> parse_3d_shapes_from_file(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    using P = shapes::Point3d<double>;
    using I = std::uint32_t;
    return stdutils::io::parse_file_generic<shapes::Soup<P,I>, char>(parse_shapes_from_stream_gen<P, I, 3>, filepath, err_handler);
}

} // namespace cdt


} // namespace io
} // namespace shapes
