#pragma once

#include <shapes/bezier.h>
#include <shapes/path_algos.h>
#include <shapes/shapes.h>
#include <stdutils/macros.h>

#include <algorithm>
#include <cassert>
#include <execution>
#include <iterator>


namespace shapes
{

template <typename F>
AllShapes<F> trivial_sampling(const AllShapes<F>& shape);

template <typename F, template<typename> typename P>
class UniformSamplingCubicBezier
{};

template <typename F>
class UniformSamplingCubicBezier<F, Point2d>
{
public:
    UniformSamplingCubicBezier(const CubicBezierPath<Point2d<F>>& cbp, bool trace_init = false);
    F max_segment_length() const;
    PointPath<Point2d<F>> sample(F max_sampling_length) const;

private:
    static constexpr unsigned int SAMPLING_BASE_N = 100;
    static constexpr unsigned int SAMPLING_ITERATIONS = 6;

    void initialize_sampling();
    void one_iteration(bool trace);

    bool m_closed_path;
    std::vector<F> m_control_points;
    std::vector<F> m_derivate_control_points;
    std::vector<float> m_sample_t;                  // Size: nb_segments * (N + 1)
    std::vector<F> m_segment_total_length;          // Size: nb_segments
    F m_max_segment_length;

    // For trace only
    struct TraceInfo
    {
        std::vector<F> dl_rel_err;
        std::vector<F> total_length;
    };
    std::vector<TraceInfo> m_trace_iterations;
};

template <typename F>
using UniformSamplingCubicBezier2d = UniformSamplingCubicBezier<F, Point2d>;

//
// Implementation
//
template <typename F>
AllShapes<F> trivial_sampling(const AllShapes<F>& shape)
{
    AllShapes<F> result;
    std::visit(stdutils::Overloaded {
        [&result](const shapes::PointCloud2d<F>& pc)        { result = pc; },
        [&result](const shapes::PointCloud3d<F>& pc)        { result = pc; },
        [&result](const shapes::PointPath2d<F>& pp)         { result = pp; },
        [&result](const shapes::PointPath3d<F>& pp)         { result = pp; },
        [&result](const shapes::CubicBezierPath2d<F>& cbp)  { result = extract_endpoints(cbp); },
        [&result](const shapes::CubicBezierPath3d<F>& cbp)  { result = extract_endpoints(cbp); },
        [&result](const shapes::Triangles2d<F>&)            { /* TBD */ },
        [&result](const shapes::Triangles3d<F>&)            { /* TBD */ },
        [](const auto&) { assert(0); }
    }, shape);
    return result;
}

template <typename F>
UniformSamplingCubicBezier<F, Point2d>::UniformSamplingCubicBezier(const CubicBezierPath<Point2d<F>>& cbp, bool trace_init)
    : m_closed_path(cbp.closed)
    , m_control_points()
    , m_derivate_control_points()
    , m_sample_t()
    , m_segment_total_length()
    , m_max_segment_length(1)
    , m_trace_iterations()
{
    assert(valid_size(cbp));

    // Copy control points, duplicate the last one if the input is a closed path
    m_control_points.reserve(6 * nb_edges(cbp) + 2);
    const F* begin_coord = reinterpret_cast<const F*>(cbp.vertices.data());
    const F* end_coord = begin_coord + (2 * cbp.vertices.size());
    std::copy(begin_coord, end_coord, std::back_inserter(m_control_points));
    if (cbp.closed)
    {
        // a valid closed CBP has at least one vertex
        m_control_points.emplace_back(cbp.vertices[0].x);
        m_control_points.emplace_back(cbp.vertices[0].y);
    }
    assert(m_control_points.size() % 2 == 0 && (m_control_points.size() / 2) % 3 == 1);

    // The derivate of a cubic bezier curve is a quadratic bezier with control points:
    //   v0 = 3 * (p1 - p0)
    //   v1 = 3 * (p2 - p1)
    //   v2 = 3 * (p3 - p2)
    const std::size_t v_sz = 6 * nb_edges(cbp);
    m_derivate_control_points.resize(v_sz, F{0});
    const F* p = m_control_points.data();
    F* v = m_derivate_control_points.data();
    // TODO: SIMD
    for (std::size_t idx = 0; idx < v_sz; idx++)
    {
        v[idx] = F{3} * (p[idx + 2] - p[idx]);
    }

    // Compute a uniform sampling of each curve segment with a high density (SAMPLING_BASE_N + 1 points, including the endpoints t = 0 and t = 1)
    initialize_sampling();
    for (unsigned int iter = 0; iter < SAMPLING_ITERATIONS; iter++)
    {
        if (trace_init)
            m_trace_iterations.emplace_back();

        // TODO replace the constant number of iterations by a convergence criteria
        one_iteration(trace_init);
    }
}

template <typename F>
F UniformSamplingCubicBezier<F, Point2d>::max_segment_length() const
{
    return m_max_segment_length;
}

template <typename F>
PointPath<Point2d<F>> UniformSamplingCubicBezier<F, Point2d>::sample(F max_sampling_length) const
{
    PointPath<Point2d<F>> result;
    result.closed = m_closed_path;

    const std::size_t nb_segs = m_derivate_control_points.size() / 6;

    result.vertices.reserve(SAMPLING_BASE_N * nb_segs + 1);

    for (std::size_t seg = 0; seg < nb_segs; seg++)
    {
        const float* const begin_sample_t = &m_sample_t[seg * (SAMPLING_BASE_N + 1)];
        assert(begin_sample_t[0] == 0.f);
        const F seg_length = m_segment_total_length[seg];
        const unsigned int nb_sampling_edges = static_cast<unsigned int>(std::ceil(seg_length / max_sampling_length)) ;    // excluding sample t = 1.f
        const F sampling_length = seg_length / static_cast<F>(nb_sampling_edges);
        const F dl = seg_length /  static_cast<F>(SAMPLING_BASE_N);
        CubicBezierMap2d<F> bezier(&m_control_points[6 * seg]);
        F cumul_l = F{0};
        for (std::size_t s = 0; s < nb_sampling_edges; s++)
        {
            float ratio_int = 0.f;
            const float ratio_frac = std::modf(static_cast<float>(cumul_l / dl), &ratio_int);
            const unsigned int idx = static_cast<unsigned int>(ratio_int);
            assert(idx < SAMPLING_BASE_N);
            result.vertices.emplace_back(bezier.at((1.f - ratio_frac) * begin_sample_t[idx] + ratio_frac * begin_sample_t[idx + 1]));
            cumul_l += sampling_length;
        }
        if (seg == nb_segs - 1 && !m_closed_path)
        {
            // Last sample of the last curve segment
            // If the curve is not a closed one, we only need to copy the last control point
            const auto last_idx = 6 * nb_segs;
            assert(m_control_points.size() == last_idx + 2);
            result.vertices.emplace_back(Vect2d<F>(m_control_points[last_idx], m_control_points[last_idx + 1]));
        }
    }
    return result;
}

template <typename F>
void UniformSamplingCubicBezier<F, Point2d>::initialize_sampling()
{
    const std::size_t nb_segs = m_derivate_control_points.size() / 6;

    m_sample_t.resize(nb_segs * (SAMPLING_BASE_N + 1));
    float dt = 1.f / static_cast<float>(SAMPLING_BASE_N);
    for (std::size_t seg = 0; seg < nb_segs; seg++)
    {
        float* begin_sample_t = &m_sample_t[seg * (SAMPLING_BASE_N + 1)];
        float t = 0.f;
        for (std::size_t idx = 0; idx < SAMPLING_BASE_N; idx++, t += dt)
        {
            *begin_sample_t++ = t;
        }
        *begin_sample_t = 1.f;
    }

    m_segment_total_length.resize(nb_segs, F{1});
    m_max_segment_length = F{1};
}

template <typename F>
void UniformSamplingCubicBezier<F, Point2d>::one_iteration(bool trace)
{
    const std::size_t nb_segs = m_derivate_control_points.size() / 6;

    std::vector<F> sample_v_norm(SAMPLING_BASE_N + 1, 0);
    std::vector<F> sample_v_norm_avg(SAMPLING_BASE_N, 0);
    std::vector<F> sample_dl(SAMPLING_BASE_N, 0);
    std::vector<float> new_sample_t(SAMPLING_BASE_N, 0);

    for (std::size_t seg = 0; seg < nb_segs; seg++)
    {
        std::size_t idx = 0;
        float* const begin_sample_t = &m_sample_t[seg * (SAMPLING_BASE_N + 1)];
        QuadraticBezierMap2d<F> derivate_bezier(&m_derivate_control_points[seg * 6]);

        // 1. Compute norm(v) and norm(v) average
        for(idx = 0; idx <= SAMPLING_BASE_N; idx++)
            sample_v_norm[idx] = norm(derivate_bezier.at(begin_sample_t[idx]));
        for(idx = 0; idx < SAMPLING_BASE_N; idx++)
            sample_v_norm_avg[idx] = F(0.5) * (sample_v_norm[idx] + sample_v_norm[idx + 1]);
        // Although the derivate of the cubic bezier can be zero, it only happens on a singular point (aka a cusp).
        // Therefore the average of the derivate between two points t0 and t1 can never be zero.
        assert(std::all_of(std::begin(sample_v_norm_avg), std::end(sample_v_norm_avg), [](const F& v_avg) { return v_avg > F{0}; }));

        // 2. Compute sample delta length, and total length
        F& seg_length = m_segment_total_length[seg];
        seg_length = F{0};
        for(idx = 0; idx < SAMPLING_BASE_N; idx++)
            seg_length += sample_dl[idx] = sample_v_norm_avg[idx] * (begin_sample_t[idx+1] - begin_sample_t[idx]);

        // 3. The target delta length (the desired curve length between two samples)
        const F target_dl = seg_length / static_cast<F>(SAMPLING_BASE_N);

        if (trace)
        {
            // 3.5 [optional] Compute error on dl with the current sampling
            F max_rel_err = F{0};
            std::for_each(sample_dl.begin(), sample_dl.end(), [&max_rel_err, target_dl](const F& dl) { max_rel_err = std::max(max_rel_err, std::abs(dl - target_dl) / target_dl); });
            m_trace_iterations.back().dl_rel_err.push_back(max_rel_err);
        }

        // 4. Compute the new sampling in t
        idx = 0;
        F cumul_l = F{0}, prev_cumul_l = F{0};
        std::size_t k = 1;
        F next_l = target_dl;
        while (k < SAMPLING_BASE_N)
        {
            while(cumul_l < next_l && idx <= SAMPLING_BASE_N)
            {
                prev_cumul_l = cumul_l;
                cumul_l += sample_dl[idx++];
            }
            if (next_l - prev_cumul_l < cumul_l - next_l)
                new_sample_t[k++] = begin_sample_t[idx - 1] + static_cast<float>((next_l - prev_cumul_l) / sample_v_norm_avg[idx - 1]);
            else
                new_sample_t[k++] = begin_sample_t[idx] - static_cast<float>((cumul_l - next_l) / sample_v_norm_avg[idx - 1]);
            assert(new_sample_t[k - 1] >= begin_sample_t[idx - 1]);
            next_l += target_dl;
        }
        assert(k == SAMPLING_BASE_N);
        std::copy(&new_sample_t[1], new_sample_t.data() + SAMPLING_BASE_N, &begin_sample_t[1]);
        assert(begin_sample_t[0] == 0.f);
        assert(begin_sample_t[SAMPLING_BASE_N] == 1.f);
    }
    if (trace)
        std::copy(std::begin(m_segment_total_length), std::end(m_segment_total_length), std::back_inserter(m_trace_iterations.back().total_length));

    m_max_segment_length = *std::max_element(std::begin(m_segment_total_length), std::end(m_segment_total_length));
}

} // namespace shapes
