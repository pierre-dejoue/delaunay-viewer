#include <svg/svg.h>

#include <ssvg_init.h>
#include <ssvg/ssvg.h>
#include <stdutils/macros.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <sstream>


namespace svg
{
namespace io
{

namespace
{
ssvg::ShapeAttributes init_default_shape_attr()
{
    ssvg::ShapeAttributes defaultAttrs;
	std::memset(&defaultAttrs, 0, sizeof(ssvg::ShapeAttributes));
    defaultAttrs.m_Parent = nullptr;
	defaultAttrs.m_StrokePaint.m_Type = ssvg::PaintType::None;
	defaultAttrs.m_StrokePaint.m_ColorABGR = 0x00000000;
	defaultAttrs.m_FillPaint.m_Type = ssvg::PaintType::None;
	defaultAttrs.m_FillPaint.m_ColorABGR = 0x00000000;
    ssvg::transformIdentity(&defaultAttrs.m_Transform[0]);
	defaultAttrs.m_StrokeMiterLimit = 4.0f;
    defaultAttrs.m_StrokeWidth = 1.0f;
	defaultAttrs.m_StrokeOpacity = 1.0f;
	defaultAttrs.m_FillOpacity = 1.0f;
    defaultAttrs.m_FontSize = 8.0f;
    //defaultAttrs.m_Opacity
    defaultAttrs.m_Flags = 0;
	defaultAttrs.m_StrokeLineJoin = ssvg::LineJoin::Miter;
    defaultAttrs.m_StrokeLineCap = ssvg::LineCap::Butt;
    defaultAttrs.m_FillRule = ssvg::FillRule::NonZero;
    //defaultAttrs.m_ID
    const std::string font_family = "sans-serif";
    std::memcpy(&defaultAttrs.m_FontFamily[0], font_family.data(), font_family.size());
    return defaultAttrs;
}


const ssvg::ShapeAttributes& get_default_shape_attributes()
{
    static ssvg::ShapeAttributes defaultAttrs = init_default_shape_attr();
    return defaultAttrs;
}

struct SSVGImageEncapsulate
{
    SSVGImageEncapsulate(ssvg::Image* img_ptr) : ptr(img_ptr) { }
    ~SSVGImageEncapsulate() { if (ptr) { ssvg::imageDestroy(ptr); } }
    ssvg::Image* const ptr;
};

template <typename F>
void parse_ssvg_image_shape_list(const ssvg::ShapeList* shape_list_ptr, Paths<F>& out_paths, const stdutils::io::ErrorHandler& err_handler);

template <typename F>
void parse_ssvg_image_path(const ssvg::Path& path, const unsigned int idx_start, const unsigned int idx_end, Paths<F>& out_paths, const stdutils::io::ErrorHandler& err_handler)
{
    assert(idx_start < idx_end);
    assert(idx_end <= path.m_NumCommands);

    // First pass to determine the type of path and if it can be imported
    bool can_import_path = true;
    unsigned int count_lineto = 0;
    unsigned int count_cubicto = 0;
    std::string WARN_msg = "";
    for (unsigned int idx = idx_start; idx < idx_end; idx++)
    {
        const auto cmd_type = path.m_Commands[idx].m_Type;
        if (idx == idx_start && cmd_type != ssvg::PathCmdType::Enum::MoveTo)
        {
            WARN_msg = "Path should start with a MoveTo command";
            can_import_path = false;
        }
        switch (cmd_type)
        {
            case ssvg::PathCmdType::Enum::MoveTo:
                break;

            case ssvg::PathCmdType::Enum::LineTo:
                count_lineto++;
                break;

            case ssvg::PathCmdType::Enum::CubicTo:
                count_cubicto++;
                break;

            case ssvg::PathCmdType::Enum::QuadraticTo:
                count_cubicto++;
                break;

            case ssvg::PathCmdType::Enum::ArcTo:
                count_cubicto++;
                // TODO Proper conversion to cubic Bezier
                err_handler(stdutils::io::Severity::WARN, "Path ArcTo converted to a straight line");
                break;

            case ssvg::PathCmdType::Enum::ClosePath:
                // Ignore on first pass
                break;

            default:
                assert(0);
                WARN_msg = "Unknown ssvg::PathCmdType::Enum";
                can_import_path = false;
                break;
        }
    }

    // Second pass to import the vertices
    if (can_import_path && count_cubicto > 0)
    {
        auto& new_cbp = out_paths.cubic_bezier_paths.emplace_back();
        new_cbp.closed = false; // By default
        for (unsigned int idx = idx_start; idx < idx_end; idx++)
        {
            const auto& cmd = path.m_Commands[idx];
            switch (cmd.m_Type)
            {
                case ssvg::PathCmdType::Enum::MoveTo:       // Data: [0] = x, [1] = y
                    assert(new_cbp.vertices.empty());
                    new_cbp.vertices.emplace_back(
                        static_cast<F>(cmd.m_Data[0]),
                        static_cast<F>(cmd.m_Data[1]));
                    break;

                case ssvg::PathCmdType::Enum::LineTo:       // Data: [0] = x, [1] = y
                {
                    // Convert straight line to a cubic bezier
                    assert(!new_cbp.vertices.empty());
                    const shapes::Point2d<F> prev_point = new_cbp.vertices.back();
                    const shapes::Point2d<F> next_point(
                        static_cast<F>(cmd.m_Data[0]),
                        static_cast<F>(cmd.m_Data[1]));
                    new_cbp.vertices.emplace_back(
                        (F{2} / F{3}) * prev_point.x + (F{1} / F{3}) * next_point.x,
                        (F{2} / F{3}) * prev_point.y + (F{1} / F{3}) * next_point.y);
                    new_cbp.vertices.emplace_back(
                        (F{1} / F{3}) * prev_point.x + (F{2} / F{3}) * next_point.x,
                        (F{1} / F{3}) * prev_point.y + (F{2} / F{3}) * next_point.y);
                    new_cbp.vertices.push_back(next_point);
                    break;
                }

                case ssvg::PathCmdType::Enum::CubicTo:      // Data: [0] = x1, [1] = y1, [2] = x2, [3] = y2, [4] = x, [5] = y
                    new_cbp.vertices.emplace_back(
                        static_cast<F>(cmd.m_Data[0]),
                        static_cast<F>(cmd.m_Data[1]));
                    new_cbp.vertices.emplace_back(
                        static_cast<F>(cmd.m_Data[2]),
                        static_cast<F>(cmd.m_Data[3]));
                    new_cbp.vertices.emplace_back(
                        static_cast<F>(cmd.m_Data[4]),
                        static_cast<F>(cmd.m_Data[5]));
                    break;

                case ssvg::PathCmdType::Enum::QuadraticTo:  // Data: [0] = x1, [1] = y1, [2] = x, [3] = y
                {
                    // Convert quad bezier to a cubic bezier
                    assert(!new_cbp.vertices.empty());
                    const shapes::Point2d<F> prev_point = new_cbp.vertices.back();
                    const shapes::Point2d<F> control_point(
                        static_cast<F>(cmd.m_Data[0]),
                        static_cast<F>(cmd.m_Data[1]));
                    const shapes::Point2d<F> next_point(
                        static_cast<F>(cmd.m_Data[2]),
                        static_cast<F>(cmd.m_Data[3]));
                    new_cbp.vertices.emplace_back(
                        (prev_point.x + F{2} * control_point.x) / F{3},
                        (prev_point.y + F{2} * control_point.y) / F{3});
                    new_cbp.vertices.emplace_back(
                        (next_point.x + F{2} * control_point.x) / F{3},
                        (next_point.y + F{2} * control_point.y) / F{3});
                    new_cbp.vertices.push_back(next_point);
                    break;
                }

                case ssvg::PathCmdType::Enum::ArcTo:        // Data: [0] = rx, [1] = ry, [2] = x-axis-rotation, [3] = large-arc-flag, [4] = sweep-flag, [5] = x, [6] = y
                {
                    // The conversion from Arc to Cubic Bezier performed by simple-svg (with flag ssvg::ImageLoadFlags::ConvertArcToCubicBezier) has been proven broken.
                    // It generates nan on some cases (for example with test file icons8-futurama-leela.svg).
                    // For now we just convert arc to straight lines
                    // TODO implement a more sensible conversion to cubic Bezier
                    const shapes::Point2d<F> prev_point = new_cbp.vertices.back();
                    const shapes::Point2d<F> next_point(
                        static_cast<F>(cmd.m_Data[5]),
                        static_cast<F>(cmd.m_Data[6]));
                    new_cbp.vertices.emplace_back(
                        (F{2} / F{3}) * prev_point.x + (F{1} / F{3}) * next_point.x,
                        (F{2} / F{3}) * prev_point.y + (F{1} / F{3}) * next_point.y);
                    new_cbp.vertices.emplace_back(
                        (F{1} / F{3}) * prev_point.x + (F{2} / F{3}) * next_point.x,
                        (F{1} / F{3}) * prev_point.y + (F{2} / F{3}) * next_point.y);
                    new_cbp.vertices.push_back(next_point);
                    break;
                }

                case ssvg::PathCmdType::Enum::ClosePath:
                    assert(!new_cbp.vertices.empty());
                    // The assumption here is that a closed path composed of cubic bezier curves is not closed by a line segment
                    // Rather, the first and and the last vertices are the same
                    new_cbp.vertices.pop_back();
                    new_cbp.closed = true;
                    break;
            }
        }
        assert(shapes::valid_size(new_cbp));
        assert(std::all_of(new_cbp.vertices.begin(), new_cbp.vertices.end(), [](const auto& p) { return shapes::isfinite(p); }));
    }
    else if (can_import_path)
    {
        auto& new_pp = out_paths.point_paths.emplace_back();
        new_pp.closed = false;  // by default
        for (unsigned int idx = idx_start; idx < idx_end; idx++)
        {
            const auto& cmd = path.m_Commands[idx];
            switch (cmd.m_Type)
            {
                case ssvg::PathCmdType::Enum::MoveTo:       // Data: [0] = x, [1] = y
                case ssvg::PathCmdType::Enum::LineTo:       // Data: [0] = x, [1] = y
                    new_pp.vertices.emplace_back(
                        static_cast<F>(cmd.m_Data[0]),
                        static_cast<F>(cmd.m_Data[1]));
                    break;

                case ssvg::PathCmdType::Enum::CubicTo:
                case ssvg::PathCmdType::Enum::QuadraticTo:
                case ssvg::PathCmdType::Enum::ArcTo:
                    assert(0);
                    break;

                case ssvg::PathCmdType::Enum::ClosePath:
                    new_pp.closed = true;
                    break;

                default:
                    assert(0);
                    break;
            }
        }
        assert(std::all_of(new_pp.vertices.begin(), new_pp.vertices.end(), [](const auto& p) { return shapes::isfinite(p); }));
    }
    else
    {
        assert(!can_import_path);
        assert(!WARN_msg.empty());
        std::ostringstream oss;
        oss << "Could not import SVG shape of type Path: " << WARN_msg;
        err_handler(stdutils::io::Severity::WARN, oss.str());
    }
}

template <typename F>
void parse_ssvg_image_paths(const ssvg::Path& path, Paths<F>& out_paths, const stdutils::io::ErrorHandler& err_handler)
{
    unsigned int idx_start = 0;
    unsigned int idx = 0;
    // There can be multiple paths: as many as there are MoveTo commands
    while (++idx < path.m_NumCommands)
    {
        if (path.m_Commands[idx].m_Type == ssvg::PathCmdType::Enum::MoveTo)
        {
            parse_ssvg_image_path(path, idx_start, idx, out_paths, err_handler);
            idx_start = idx;
        }
    }
    parse_ssvg_image_path(path, idx_start, path.m_NumCommands, out_paths, err_handler);
}

template <typename F>
void parse_ssvg_image_shape(const ssvg::Shape* shape_ptr, Paths<F>& out_paths, const stdutils::io::ErrorHandler& err_handler)
{
    switch (shape_ptr->m_Type)
    {
        case ssvg::ShapeType::Enum::Group:
            parse_ssvg_image_shape_list(&shape_ptr->m_ShapeList, out_paths, err_handler);
            break;

        case ssvg::ShapeType::Enum::Rect:
            err_handler(stdutils::io::Severity::WARN, "Ignored a SVG shape of type Rect");
            break;

        case ssvg::ShapeType::Enum::Circle:
            err_handler(stdutils::io::Severity::WARN, "Ignored a SVG shape of type Circle");
            break;

        case ssvg::ShapeType::Enum::Ellipse:
            err_handler(stdutils::io::Severity::WARN, "Ignored a SVG shape of type Ellipse");
            break;

        case ssvg::ShapeType::Enum::Line:
        {
            auto& new_pp = out_paths.point_paths.emplace_back();
            new_pp.closed = false;
            new_pp.vertices.emplace_back(
                static_cast<F>(shape_ptr->m_Line.x1),
                static_cast<F>(shape_ptr->m_Line.y1));
            new_pp.vertices.emplace_back(
                static_cast<F>(shape_ptr->m_Line.x2),
                static_cast<F>(shape_ptr->m_Line.y2));
            break;
        }

        case ssvg::ShapeType::Enum::Polyline:
        case ssvg::ShapeType::Enum::Polygon:
        {
            auto& new_pp = out_paths.point_paths.emplace_back();
            new_pp.closed = (shape_ptr->m_Type == ssvg::ShapeType::Enum::Polygon);
            for (unsigned int idx = 0; idx < shape_ptr->m_PointList.m_NumPoints; idx++)
            {
                new_pp.vertices.emplace_back(
                    static_cast<F>(shape_ptr->m_PointList.m_Coords[2*idx]),
                    static_cast<F>(shape_ptr->m_PointList.m_Coords[2*idx + 1]));
            }
            assert(std::all_of(new_pp.vertices.begin(), new_pp.vertices.end(), [](const auto& p) { return shapes::isfinite(p); }));
            break;
        }

        case ssvg::ShapeType::Enum::Path:
            parse_ssvg_image_paths(shape_ptr->m_Path, out_paths, err_handler);
            break;

        case ssvg::ShapeType::Enum::Text:
            err_handler(stdutils::io::Severity::WARN, "Ignored a SVG shape of type Text");
            break;

        default:
            assert(0);
            err_handler(stdutils::io::Severity::ERR, "Unknown ssvg::ShapeType::Enum");
            break;
    }
}

template <typename F>
void parse_ssvg_image_shape_list(const ssvg::ShapeList* shape_list_ptr, Paths<F>& out_paths, const stdutils::io::ErrorHandler& err_handler)
{
    for (unsigned int idx = 0; idx < shape_list_ptr->m_NumShapes; idx++)
    {
        parse_ssvg_image_shape(&shape_list_ptr->m_Shapes[idx], out_paths, err_handler);
    }
}

template <typename F>
Paths<F> parse_svg_paths_gen(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    Paths<F> result;
    try
    {
        const auto sz = std::filesystem::file_size(filepath);
        std::vector<char> buf(sz + 1, 0u);
        {
            std::ifstream inputstream(filepath);
            if (inputstream.is_open())
            {
                inputstream.read(buf.data(), static_cast<std::streamsize>(sz));
            }
            else
            {
                std::stringstream oss;
                oss << "Cannot open file " << filepath;
                err_handler(stdutils::io::Severity::FATAL, oss.str());
            }
        }
        constexpr std::uint32_t svg_parser_flags = 0;
        initialize_ssvg_lib();
        SSVGImageEncapsulate ssvg_img(ssvg::imageLoad(buf.data(), svg_parser_flags, &get_default_shape_attributes()));
        if (ssvg_img.ptr)
            parse_ssvg_image_shape_list(&ssvg_img.ptr->m_ShapeList, result, err_handler);
        else
            err_handler(stdutils::io::Severity::ERR, "Library simple-svg failed to parse the image");
    }
    catch(const std::exception& e)
    {
        std::stringstream oss;
        oss << "Exception: " << e.what();
        err_handler(stdutils::io::Severity::EXCPT, oss.str());
    }

    return result;
}

} // Anonymous namespace

Paths<double> parse_svg_paths(std::filesystem::path filepath, const stdutils::io::ErrorHandler& err_handler) noexcept
{
    return parse_svg_paths_gen<double>(filepath, err_handler);
}

} // namespace io
} // namespace svg
