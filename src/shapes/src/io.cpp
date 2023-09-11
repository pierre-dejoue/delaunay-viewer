#include <shapes/io.h>

#include <shapes/traits.h>
#include <stdutils/io.h>
#include <stdutils/string.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
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
    {
        entry[idx] = *numeric_stream++;
        idx++;      // postincrement outside of the previous expression due to a warning C28020 false positive
    }
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
    std::size_t line_nb_start;
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


/**
 * DAT format description
 *
 * The file is in text format, read line by line as follows.
 *
 * - Empty lines anywhere in the file are ignored.
 * - Lines starting with '#' are comment lines, and they are also ignored.
 * - The other lines fall into two categories:
 *      - point lines (e.g  "1.000 2.000" or "1.000 2.000 3.000") representing the coordinates of a 2D or 3D point
 *      - control lines (e.g. "LINE_PATH" or "CUBIC_BEZIER_PATH OPEN") describing the type of the next point series
 * - Point series are a sequence of point lines uninterrupted by a control line.
 * - A control lines comprises of two tokens: <shape_type> <shape_topology>, where:
 *      - <shape_type> can be (the case is ignored): "POINT_CLOUD", "POINT_PATH", "CUBIC_BEZIER_PATH"
 *      - <shape_topology> is optional, it can be (the case is ignored): "OPEN", "CLOSED" (the default).
 * - Any line in the file that is not a well-formed control line, a comment or a blank line is considered a separation
 *   between point series. In the case of a malformed control line, the next series is considered to be a POINT_CLOUD.
 * - Control lines are optional. In the absence of any control line, the file consists of a single point series
 *   considered to be a POINT_CLOUD.
 */
namespace dat
{
namespace
{

template <typename F, int POINT_DIM, std::size_t MAX_DIM>
void append_new_shape(const ShapeBuffer<F, MAX_DIM>& buffer, ShapeAggregate<F>& shapes, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    static_assert(static_cast<std::size_t>(POINT_DIM) <= MAX_DIM);
    switch (buffer.type)
    {
        case ShapeType::POINT_CLOUD:
        {
            shapes::PointCloud<typename shapes::Traits<F, POINT_DIM>::Point> pc;
            append_vertices(pc.vertices, buffer.vertices);
            shapes.emplace_back(std::move(pc));
            break;
        }
        case ShapeType::POINT_PATH:
        {
            shapes::PointPath<typename shapes::Traits<F, POINT_DIM>::Point> pp;
            pp.closed = buffer.closed;
            append_vertices(pp.vertices, buffer.vertices);
            shapes.emplace_back(std::move(pp));
            break;
        }
        case ShapeType::CUBIC_BEZIER_PATH:
        {
            shapes::CubicBezierPath<typename shapes::Traits<F, POINT_DIM>::Point> cbp;
            cbp.closed = buffer.closed;
            append_vertices(cbp.vertices, buffer.vertices);
            if (shapes::valid_size(cbp))
            {
                shapes.emplace_back(std::move(cbp));
            }
            else
            {
                std::stringstream out;
                out << "Ignored a cubic bezier path (started line " << buffer.line_nb_start << ") with invalid size " << cbp.vertices.size();
                err_handler(stdutils::io::Severity::WARN, out.str());
            }
            break;
        }
    }
}

template <typename F>
ShapeAggregate<F> parse_shapes_from_stream_gen(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler)
{
    ShapeAggregate<F> result;
    std::string line;
    ShapeBuffer<F, 3> buffer;
    auto linestream = stdutils::io::SkipLineStream(inputstream).skip_blank_lines().skip_comment_lines("#");
    while (linestream.stream().good())
    {
        // Read point series
        while (linestream.getline(line) && parse_numeric_line(line, buffer.vertices)) {}

        // Control line, or EOF
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
        buffer.line_nb_start = linestream.line_nb();
        // 3. Type of the next shape
        TokenIterator token_iterator(line);
        const std::string first_token = token_iterator.next_token();
        const auto type_str = stdutils::string::tolower(first_token);
        if (type_str == "point_path") { buffer.type = ShapeType::POINT_PATH; }
        else if (type_str == "cubic_bezier_path") { buffer.type = ShapeType::CUBIC_BEZIER_PATH; }
        else { buffer.type = ShapeType::POINT_CLOUD; }
        // 4. Topology of the next shape
        const auto topo_str = stdutils::string::tolower(token_iterator.next_token());
        buffer.closed = (topo_str != "open");
    }
    return result;
}

template <typename F>
struct StreamWriterInput
{
    StreamWriterInput(const ShapeAggregate<F>& shapes, char sep = '\n') : shapes(shapes), sep(sep) {}
    const ShapeAggregate<F>& shapes;
    char sep;
};

template <typename F>
void save_shapes_as_stream_gen(std::ostream& out, const StreamWriterInput<F>& input, const stdutils::io::ErrorHandler& err_handler)
{
    const auto initial_fp_digits = stdutils::io::accurate_fp_precision<F>(out);
    const char sep = input.sep;
    for (const auto& shape : input.shapes)
    {
        std::visit(stdutils::Overloaded {
            [&out, sep](const shapes::PointCloud2d<F>& pc) {
                out << "POINT_CLOUD\n";
                for (const auto& p : pc.vertices) { out << p.x << ' ' << p.y << sep; }
            },
            [&out, sep](const shapes::PointCloud3d<F>& pc) {
                out << "POINT_CLOUD\n";
                for (const auto& p : pc.vertices) { out << p.x << ' ' << p.y << ' ' << p.z << sep; }
            },
            [&out, sep](const shapes::PointPath2d<F>& pp) {
                out << "POINT_PATH " << (pp.closed ? "CLOSED" : "OPEN") << sep;
                for (const auto& p : pp.vertices) { out << p.x << ' ' << p.y << sep; }
            },
            [&out, sep](const shapes::PointPath3d<F>& pp) {
                out << "POINT_PATH " << (pp.closed ? "CLOSED" : "OPEN") << sep;
                for (const auto& p : pp.vertices) { out << p.x << ' ' << p.y << ' ' << p.z << sep; }
            },
            [&out, sep](const shapes::CubicBezierPath2d<F>& cbp) {
                out << "CUBIC_BEZIER_PATH " << (cbp.closed ? "CLOSED" : "OPEN") << sep;
                for (const auto& p : cbp.vertices) { out << p.x << ' ' << p.y << sep; }
            },
            [&out, sep](const shapes::CubicBezierPath3d<F>& cbp) {
                out << "CUBIC_BEZIER_PATH " << (cbp.closed ? "CLOSED" : "OPEN") << sep;
                for (const auto& p : cbp.vertices) { out << p.x << ' ' << p.y << ' ' << p.z << sep; }
            },
            [&err_handler](const shapes::Triangles2d<F>&) { err_handler(stdutils::io::Severity::WARN, "Shape Triangles2d not written to DAT stream"); },
            [&err_handler](const shapes::Triangles3d<F>&) { err_handler(stdutils::io::Severity::WARN, "Shape Triangles3d not written to DAT stream"); },
            [](const auto&) { assert(0); }
        }, shape);
    }
    out << std::setprecision(initial_fp_digits);
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
    return stdutils::io::open_and_parse_file<ShapeAggregate<double>, char>(filepath, parse_shapes_from_stream_gen<double>, err_handler);
}

void save_shapes_as_stream(std::ostream& outputstream, const ShapeAggregate<double>& shapes, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    try
    {
        save_shapes_as_stream_gen<double>(outputstream, StreamWriterInput(shapes), err_handler);
    }
    catch (const std::exception& e)
    {
        std::stringstream oss;
        oss << "Exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, oss.str());
    }
}

void save_shapes_as_file(std::filesystem::path filepath, const ShapeAggregate<double>& shapes, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    stdutils::io::save_file<StreamWriterInput<double>, char>(filepath, save_shapes_as_stream_gen<double>, StreamWriterInput(shapes), err_handler);
}

void save_shapes_as_oneliner_stream(std::ostream& outputstream, const ShapeAggregate<double>& shapes, std::string_view postfix) noexcept
{
    const stdutils::io::ErrorHandler err_handler = [&outputstream](stdutils::io::SeverityCode code, stdutils::io::ErrorMessage msg)
    {
        outputstream << stdutils::io::str_severity_code(code) << ": " << msg << ' ';
    };
    try
    {
        save_shapes_as_stream_gen<double>(outputstream, StreamWriterInput(shapes, ' '), err_handler);
    }
    catch (const std::exception& e)
    {
        outputstream << "Exception: " << e.what();
    }
    outputstream << postfix;
}

} // namespace dat

/**
 * CDT format description
 *
 * The CDT format describes a geometry composed of one point cloud, one edge soup and one triangle soup. (Each of which can be empty.)
 *
 * The file is in text format, read line by line as follows.
 *
 * - Empty lines anywhere in the file are ignored.
 * - Lines starting with '#' are comment lines, and they are also ignored.
 * - The geometry description comprises of four consecutive sections:
 *      - A HEADER section (one line) indicating the expected number of vertices, edges and triangles.
 *      - A VERTEX section with one vertex per line (e.g  "1.000 2.000" or "1.000 2.000 3.000").
 *      - A EDGE section with one pair of vertex indices per line (e.g. "1 2").
 *      - A TRIANGLE section with one triplet of vertex indices per line (e.g "1 0 2").
 * - The EDGE and TRIANGLE indices refer to the vertices of the VERTEX section, starting with index 0.
 * - A vertex can be referenced by several edges and triangles.
 * - Any vertex that is not referenced by either an edge or a triangle is considered to be part of the point cloud.
 * - The HEADER has the following format: <nb_vertices> <nb_edges> <nb_triangles>. Only the nb of vertices is mandatory, others default to 0.
 *      - E.g. "5 3" for 5 vertices, 3 edges and no triangles
 *      - E.g. "5 1 2" for 5 vertices, 1 edge and 2 triangles
 *      - E.g. "6" for 6 vertices, no edges and no triangles (the geometry is simply a point cloud)
 * - The number of lines of the VERTEX section is expected to be exactly equal to <nb_vertices>.
 * - The number of lines of the EDGE (resp. TRIANGLE) section is expected to be exactly equal to <nb_edges> (resp. <nb_triangles>).
 */
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

template <typename I>
void warn_eliminated_edge(const graphs::Edge<I>& edge, std::string_view reason, const stdutils::io::ErrorHandler& err_handler)
{
    std::stringstream out;
    out << "Eliminated invalid edge [ " << edge.first << ", " << edge.second << " ]: " << reason;
    err_handler(stdutils::io::Severity::WARN, out.str());
}

template <typename I>
void warn_eliminated_triangle(const graphs::Triangle<I>& tri, std::string_view reason, const stdutils::io::ErrorHandler& err_handler)
{
    std::stringstream out;
    out << "Eliminated invalid triangle [ " << tri[0] << ", " << tri[1] << ", " << tri[2] << " ]: " << reason;
    err_handler(stdutils::io::Severity::WARN, out.str());
}

enum class CDT_State
{
    HeaderLine,
    ParseVertices,
    ParseEdgeIndices,
    ParseTriangleIndices,
    Done
};

template <typename F, typename I>
unsigned int peek_point_dimension_gen(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler)
{
    unsigned int result = 0;
    CDT_State cdt_state = CDT_State::HeaderLine;
    auto linestream = stdutils::io::SkipLineStream(inputstream).skip_blank_lines().skip_comment_lines("#");
    while (linestream.stream().good() && cdt_state != CDT_State::Done)
    {
        std::string line;
        switch (cdt_state)
        {
            case CDT_State::HeaderLine:
            {
                NumericLineBuffer<I, 3> count_buffer;
                while (linestream.getline(line) && !parse_numeric_line(line, count_buffer))
                {
                    std::stringstream out;
                    out << "CDT_State: HeaderLine. Invalid line (" << linestream.line_nb() << ") was skipped.";
                    err_handler(stdutils::io::Severity::WARN, out.str());
                }
                if (count_buffer.lines.size() == 1)
                {
                    cdt_state = CDT_State::ParseVertices;
                }
                else
                {
                    err_handler(stdutils::io::Severity::ERR, "CDT_State: HeaderLine. Could not find the header line.");
                    cdt_state = CDT_State::Done;
                }
                break;
            }
            case CDT_State::ParseVertices:
            {
                NumericLineBuffer<F, 3> vertex_buffer;
                while (vertex_buffer.lines.size() < 1 && linestream.getline(line) && parse_numeric_line(line, vertex_buffer)) { }
                result = vertex_buffer.dim;
                cdt_state = CDT_State::Done;
                break;
            }
            case CDT_State::ParseEdgeIndices:
            case CDT_State::ParseTriangleIndices:
            case CDT_State::Done:
            default:
                assert(0);
                break;
        }
    }
    if (result == 0)
    {
        err_handler(stdutils::io::Severity::ERR, "Could not deduce the point dimension");
    }
    if (result != 2 && result != 3)
    {
        std::stringstream out;
        out << "Invalid point dimension: " << result;
        err_handler(stdutils::io::Severity::ERR, out.str());
    }
    return result;
}

template <typename P, typename I>
shapes::Soup<P, I> parse_shapes_from_stream_gen(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler)
{
    using F = typename P::scalar;
    constexpr auto POINT_DIM = static_cast<std::size_t>(P::dim);
    shapes::Soup<P, I> result;
    CDT_State cdt_state = CDT_State::HeaderLine;
    I nb_vertices = 0;
    I nb_edges = 0;
    I nb_triangles = 0;
    std::vector<P> vertices;
    graphs::EdgeSoup<I> edges;
    graphs::TriangleSoup<I> triangles;
    constexpr I undef = graphs::IndexTraits<I>::undef();
    auto linestream = stdutils::io::SkipLineStream(inputstream).skip_blank_lines().skip_comment_lines("#");
    while (linestream.stream().good() && cdt_state != CDT_State::Done)
    {
        std::string line;
        switch (cdt_state)
        {
            case CDT_State::HeaderLine:
            {
                NumericLineBuffer<I, 3> count_buffer;
                while (linestream.getline(line) && !parse_numeric_line(line, count_buffer))
                {
                    std::stringstream out;
                    out << "CDT_State: HeaderLine. Invalid line (" << linestream.line_nb() << ") was skipped.";
                    err_handler(stdutils::io::Severity::WARN, out.str());
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
                break;
            }
            case CDT_State::ParseVertices:
            {
                NumericLineBuffer<F, POINT_DIM> vertex_buffer;
                while (vertex_buffer.lines.size() < nb_vertices && linestream.getline(line) && parse_numeric_line(line, vertex_buffer)) { }
                append_vertices(vertices, vertex_buffer);
                cdt_state = CDT_State::ParseEdgeIndices;
                break;
            }
            case CDT_State::ParseEdgeIndices:
            {
                NumericLineBuffer<I, 2> edge_buffer;
                while (edge_buffer.lines.size() < nb_edges && linestream.getline(line) && parse_numeric_line(line, edge_buffer, undef)) { }
                edges.reserve(nb_edges);
                for (const auto& arr : edge_buffer.lines) { edges.emplace_back(arr[0], arr[1]); }
                cdt_state = CDT_State::ParseTriangleIndices;
                break;
            }
            case CDT_State::ParseTriangleIndices:
            {
                NumericLineBuffer<I, 3> triangle_buffer;
                while (triangle_buffer.lines.size() < nb_triangles && linestream.getline(line) && parse_numeric_line(line, triangle_buffer, undef)) { }
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
            if (e.first == e.second) { warn_eliminated_edge(e, "loop edge", err_handler); return false; }
            if (e.first >= nb_vertices || e.second >= nb_vertices) { warn_eliminated_edge(e, "out of bound index", err_handler); return false; }
            if (!ordered_edges.insert(graphs::ordered_edge(e)).second) { warn_eliminated_edge(e, "duplicated edge", err_handler); return false; }
            return true;
        });
        assert(graphs::is_valid(result.edges.indices));
    }
    {
        result.triangles.faces.reserve(triangles.size());
        std::copy_if(triangles.cbegin(), triangles.cend(), std::back_inserter(result.triangles.faces), [&err_handler, nb_vertices](const auto& t) {
            if (!graphs::is_valid(t)) { warn_eliminated_triangle(t, "repeat index", err_handler); return false; }
            if (t[0] >= nb_vertices || t[1] >= nb_vertices || t[2] >= nb_vertices) { warn_eliminated_triangle(t, "out of bound index", err_handler); return false; }
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

} // Anonymous namespace

unsigned int peek_point_dimension(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    try
    {
        return peek_point_dimension_gen<double, std::uint32_t>(inputstream, err_handler);
    }
    catch (const std::exception& e)
    {
        std::stringstream oss;
        oss << "Exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, oss.str());
    }
    return 0;
}

unsigned int peek_point_dimension(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    return stdutils::io::open_and_parse_file<unsigned int, char>(filepath, peek_point_dimension_gen<double, std::uint32_t>, err_handler);
}

shapes::Soup2d<double> parse_2d_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    using P = shapes::Point2d<double>;
    using I = std::uint32_t;
    try
    {
        return parse_shapes_from_stream_gen<P, I>(inputstream, err_handler);
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
    return stdutils::io::open_and_parse_file<shapes::Soup<P,I>, char>(filepath, parse_shapes_from_stream_gen<P, I>, err_handler);
}

shapes::Soup3d<double> parse_3d_shapes_from_stream(std::istream& inputstream, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    using P = shapes::Point3d<double>;
    using I = std::uint32_t;
    try
    {
        return parse_shapes_from_stream_gen<P, I>(inputstream, err_handler);
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
    return stdutils::io::open_and_parse_file<shapes::Soup<P,I>, char>(filepath, parse_shapes_from_stream_gen<P, I>, err_handler);
}

} // namespace cdt


} // namespace io
} // namespace shapes
