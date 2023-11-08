#pragma once

#include <shapes/bezier.h>
#include <shapes/path_algos.h>
#include <shapes/sampling_interface.h>
#include <shapes/shapes.h>
#include <stdutils/algorithm.h>
#include <stdutils/stats.h>

#include <algorithm>
#include <cassert>
#include <execution>
#include <iterator>


namespace shapes
{

/**
 *  Trivial sampling just extracts the endpoints of each path
 */
template <typename F>
AllShapes<F> trivial_sampling(const AllShapes<F>& shape);

/**
 * Uniform sampling of point paths
 */
template <typename F, template<typename> typename P>
class UniformSamplingPointPath : public UniformSamplingInterface<F, P>
{
public:
    UniformSamplingPointPath(const PointPath<P<F>>& pp);
    ~UniformSamplingPointPath() = default;
    F max_segment_length() const override;
    PointPath<P<F>> sample(F max_sampling_length) const override;

private:
    PointPath<P<F>> m_point_path;
    std::vector<F> m_segment_length;
    F m_max_segment_length;
};

template <typename F>
using UniformSamplingPointPath2d = UniformSamplingPointPath<F, Point2d>;

/**
 * Uniform sampling of cubic bezier paths
 */
template <typename F, template<typename> typename P>
class UniformSamplingCubicBezier : public UniformSamplingInterface<F, P>
{};

template <typename F>
class UniformSamplingCubicBezier<F, Point2d> : public UniformSamplingInterface<F, Point2d>
{
public:
    // For debug trace only
    struct InitIterationTraceInfo
    {
        std::vector<F> dl_max_relative_error;
        std::vector<F> edge_length_relative_range;
        std::vector<F> total_length;
    };
    struct InitTraceInfo
    {
        std::vector<InitIterationTraceInfo> iterations;
        std::vector<std::size_t> nb_edges_w_quadratic_arc_model;
    };

    UniformSamplingCubicBezier(const CubicBezierPath<Point2d<F>>& cbp, InitTraceInfo* trace_info = nullptr);
    ~UniformSamplingCubicBezier() = default;
    F max_segment_length() const override;
    PointPath<Point2d<F>> sample(F max_sampling_length) const override;

private:
    static constexpr unsigned int SAMPLING_BASE_N = 100;
    static constexpr unsigned int SAMPLING_ITERATIONS = 6;
    // Limit on the max relative length error above which we switch to a more precise model of the arc length between two samples
    static constexpr float QUADRATIC_ARC_MODEL_RELATIVE_LENGTH_ERROR = 0.04f;

    void initialization_prepare_data_structures();
    void initialization_one_iteration(InitIterationTraceInfo* iter_trace_info);
    void initialization_finalize(InitTraceInfo* trace_info);

    bool m_closed_path;
    std::vector<F> m_control_points;
    std::vector<F> m_derivate_control_points;
    std::vector<float> m_sample_t;                  // Size: nb_segments * (N + 1)
    std::vector<F> m_norm_v_at_sample;              // Size: nb_segments * (N + 1)
    std::vector<float> m_max_relative_length_error; // Size: nb_segments * N
    std::vector<F> m_segment_total_length;          // Size: nb_segments
    F m_max_segment_length;
};

template <typename F>
using UniformSamplingCubicBezier2d = UniformSamplingCubicBezier<F, Point2d>;

/**
 * Output statistics related to how uniform a point path is. The result is normalized on the average edge length
 */
template <typename F, template<typename> typename P>
stdutils::stats::Result<F> path_normalized_uniformity_stats(const shapes::PointPath<P<F>>& pp);


//
//
// Implementation
//
//


template <typename F, template<typename> typename P>
UniformSamplingPointPath<F, P>::UniformSamplingPointPath(const PointPath<P<F>>& pp)
    : m_point_path(pp)
    , m_segment_length()
    , m_max_segment_length(F{0})
{
    m_segment_length.reserve(nb_edges(pp));
    const auto sz = pp.vertices.size();
    for (std::size_t idx = 0; idx < sz; idx++)
    {
        if (!pp.closed && idx == sz - 1)
            break;
        const P<F>& p0 = pp.vertices[idx];
        const P<F>& p1 = pp.vertices[(idx + 1) % sz];
        const auto& length = m_segment_length.emplace_back(shapes::norm(p1 - p0));
        stdutils::max_update(m_max_segment_length, length);
    }
}

template <typename F, template<typename> typename P>
F UniformSamplingPointPath<F, P>::max_segment_length() const
{
    return m_max_segment_length;
}

template <typename F, template<typename> typename P>
PointPath<P<F>> UniformSamplingPointPath<F, P>::sample(F max_sampling_length) const
{
    PointPath<P<F>> result;
    result.closed = m_point_path.closed;

    // TODO result.vertices.reserve( ? );
    const auto sz = m_point_path.vertices.size();
    for (std::size_t idx = 0; idx < sz; idx++)
    {
        if (!m_point_path.closed && idx == sz - 1)
            break;
        const P<F>& p0 = m_point_path.vertices[idx];
        const P<F>& p1 = m_point_path.vertices[(idx + 1) % sz];
        const F seg_length = m_segment_length[idx];
        const unsigned int nb_sampling_edges = static_cast<unsigned int>(std::ceil(seg_length / max_sampling_length)) ;    // excluding sample t = 1.f
        const F dt = F{1} / static_cast<F>(nb_sampling_edges);
        F t{0};
        for (std::size_t s = 0; s < nb_sampling_edges; s++, t += dt)
        {
            result.vertices.emplace_back((F{1} - t) * p0 + t * p1);
        }
    }
    if (!m_point_path.closed)
    {
        // If the curve is not a closed one, we need to copy the last control point
        result.vertices.emplace_back(m_point_path.vertices.back());
    }

    return result;
}

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
        [&result](const shapes::Edges2d<F>& es)             { result = es; },
        [&result](const shapes::Edges3d<F>& es)             { result = es; },
        [](const shapes::Triangles2d<F>&)                   { /* TBD */ },
        [](const shapes::Triangles3d<F>&)                   { /* TBD */ },
        [](const auto&) { assert(0); }
    }, shape);
    return result;
}

template <typename F>
UniformSamplingCubicBezier<F, Point2d>::UniformSamplingCubicBezier(const CubicBezierPath<Point2d<F>>& cbp, InitTraceInfo* trace_info)
    : m_closed_path(cbp.closed)
    , m_control_points()
    , m_derivate_control_points()
    , m_sample_t()
    , m_norm_v_at_sample()
    , m_max_relative_length_error()
    , m_segment_total_length()
    , m_max_segment_length{1}
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
    initialization_prepare_data_structures();
    InitIterationTraceInfo* iteration_trace_info = nullptr;
    for (unsigned int iter = 0; iter < SAMPLING_ITERATIONS; iter++)
    {
        if (trace_info)
            iteration_trace_info = &trace_info->iterations.emplace_back();

        // TODO replace the constant number of iterations by a convergence criteria
        initialization_one_iteration(iteration_trace_info);
    }
    initialization_finalize(trace_info);
}

template <typename F>
void UniformSamplingCubicBezier<F, Point2d>::initialization_prepare_data_structures()
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

    m_norm_v_at_sample.resize(nb_segs * (SAMPLING_BASE_N + 1));
    m_max_relative_length_error.resize(nb_segs * SAMPLING_BASE_N);
    m_segment_total_length.resize(nb_segs, F{1});
    m_max_segment_length = F{1};
}

template <typename F>
void UniformSamplingCubicBezier<F, Point2d>::initialization_one_iteration(InitIterationTraceInfo* iter_trace_info)
{
    const std::size_t nb_segs = m_derivate_control_points.size() / 6;

    std::vector<F> sample_v_norm(SAMPLING_BASE_N + 1, 0);
    std::vector<F> sample_v_norm_avg(SAMPLING_BASE_N, 0);
    std::vector<F> sample_dl(SAMPLING_BASE_N, 0);
    std::vector<float> new_sample_t(SAMPLING_BASE_N, 0);

    for (std::size_t seg = 0; seg < nb_segs; seg++)
    {
        float* const begin_sample_t = &m_sample_t[seg * (SAMPLING_BASE_N + 1)];
        QuadraticBezierMap2d<F> derivate_bezier(&m_derivate_control_points[seg * 6]);

        // 1. Compute norm(v) and norm(v) average
        std::size_t idx = 0;
        for(idx = 0; idx <= SAMPLING_BASE_N; idx++)
            sample_v_norm[idx] = norm(derivate_bezier.at(begin_sample_t[idx]));
        for(idx = 0; idx < SAMPLING_BASE_N; idx++)
            sample_v_norm_avg[idx] = F(0.5) * (sample_v_norm[idx] + sample_v_norm[idx + 1]);
        // Although the derivate of the cubic bezier can be zero, it only happens on a singular point (aka a cusp).
        // Therefore the average of the derivate between two points t0 and t1 can never be zero.
        assert(std::all_of(std::cbegin(sample_v_norm_avg), std::cend(sample_v_norm_avg), [](const F& v_avg) { return v_avg > F{0}; }));

        // 2. Compute sample delta length, and total length
        F& seg_length = m_segment_total_length[seg];
        seg_length = F{0};
        for(idx = 0; idx < SAMPLING_BASE_N; idx++)
            seg_length += sample_dl[idx] = sample_v_norm_avg[idx] * static_cast<F>(begin_sample_t[idx+1] - begin_sample_t[idx]);

        // 3. The target delta length (the desired curve length between two samples)
        const F target_dl = seg_length / static_cast<F>(SAMPLING_BASE_N);
        assert(std::fpclassify(target_dl) != FP_ZERO);

        if (iter_trace_info)
        {
            // 3.5 Compute error on the target dl with the current sampling
            stdutils::stats::CumulSamples<F> sampling;
            sampling.add_samples(sample_dl.begin(), sample_dl.end());
            const auto normalized_stats = sampling.get_result().normalize_to(target_dl);
            iter_trace_info->dl_max_relative_error.emplace_back(std::max(std::abs(F{1} - normalized_stats.min), std::abs(normalized_stats.max - F{1})));
            iter_trace_info->total_length.emplace_back(seg_length);
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

        if (iter_trace_info)
        {
            shapes::PointPath2d<F> pp;
            pp.closed = false;
            pp.vertices.reserve(SAMPLING_BASE_N + 1);
            CubicBezierMap2d<F> bezier(&m_control_points[6 * seg]);
            std::transform(&begin_sample_t[0], &begin_sample_t[0] + SAMPLING_BASE_N + 1, std::back_insert_iterator(pp.vertices), [&bezier](const float t) { return bezier.at(t); });
            const auto normalized_stats = path_normalized_uniformity_stats(pp);
            iter_trace_info->edge_length_relative_range.emplace_back(normalized_stats.range);
        }
    }

    m_max_segment_length = *std::max_element(std::cbegin(m_segment_total_length), std::cend(m_segment_total_length));
}

template <typename F>
void UniformSamplingCubicBezier<F, Point2d>::initialization_finalize(InitTraceInfo* trace_info)
{
    const std::size_t nb_segs = m_derivate_control_points.size() / 6;
    for (std::size_t seg = 0; seg < nb_segs; seg++)
    {
        float* const begin_sample_t = &m_sample_t[seg * (SAMPLING_BASE_N + 1)];
        F* const begin_norm_v = &m_norm_v_at_sample[seg * (SAMPLING_BASE_N + 1)];
        float* const begin_max_rel_err = &m_max_relative_length_error[seg * SAMPLING_BASE_N];
        QuadraticBezierMap2d<F> derivate_bezier(&m_derivate_control_points[seg * 6]);

        // 1. Compute norm(v) at each sample point
        std::size_t idx = 0;
        for(idx = 0; idx <= SAMPLING_BASE_N; idx++)
            begin_norm_v[idx] = norm(derivate_bezier.at(begin_sample_t[idx]));

        // 2. Evaluate the max error made on the arc length between two consecutive samples if we assume a constant average v_norm between those
        for(idx = 0; idx < SAMPLING_BASE_N; idx++)
        {
            const F v0 = begin_norm_v[idx];
            const F v1 = begin_norm_v[idx + 1];
            if(v0 > F{0} || v1 > F{0})
            {
                // By construction always comprised between 0 and 2
                begin_max_rel_err[idx] = static_cast<float>(F{2} * std::abs(v1 - v0) / (v0 + v1));
            }
            // else, keep the defaut zero initialization
        }

        if (trace_info)
        {
            trace_info->nb_edges_w_quadratic_arc_model.emplace_back(std::count_if(begin_max_rel_err, begin_max_rel_err + SAMPLING_BASE_N,
                [](const auto max_rel_err) { return max_rel_err > QUADRATIC_ARC_MODEL_RELATIVE_LENGTH_ERROR; }));
        }
    }
}

template <typename F>
F UniformSamplingCubicBezier<F, Point2d>::max_segment_length() const
{
    return m_max_segment_length;
}

namespace
{

// Given the arc length derivates at t=0, v0, and at t=1, v1, and given a length_ratio comprised between 0 and 1,
// returns a time ratio comprised between 0 and 1, which is the solution of the quadratic model of the arc length
template <typename F>
F precise_time_ratio(F v0, F v1, F length_ratio)
{
    assert(v0 >= F{0});
    assert(v1 >= F{0});
    assert(std::abs(v0 - v1) / (v0 + v1) > static_cast<F>(1e-05));      // The quadratic model is not relevant when v0 and v1 are very close
    const F lr = std::clamp(length_ratio, F{0}, F{1});
    return std::clamp((v0 - std::sqrt(v0 *v0 * (F{1}- lr) + v1 * v1 * lr)) / (v0 - v1) , F{0}, F{1});
}

} // anonymous namespace

template <typename F>
PointPath<Point2d<F>> UniformSamplingCubicBezier<F, Point2d>::sample(F max_sampling_length) const
{
    PointPath<Point2d<F>> result;
    result.closed = m_closed_path;

    const std::size_t nb_segs = m_derivate_control_points.size() / 6;

    // TODO result.vertices.reserve( ? );

    for (std::size_t seg = 0; seg < nb_segs; seg++)
    {
        const float* const begin_sample_t = &m_sample_t[seg * (SAMPLING_BASE_N + 1)];
        const F* const begin_norm_v = &m_norm_v_at_sample[seg * (SAMPLING_BASE_N + 1)];
        const float* const begin_max_rel_err = &m_max_relative_length_error[seg * SAMPLING_BASE_N];
        assert(begin_sample_t[0] == 0.f);
        const F seg_length = m_segment_total_length[seg];
        const unsigned int nb_sampling_edges = static_cast<unsigned int>(std::ceil(seg_length / max_sampling_length)) ;    // excluding sample t = 1.f
        const F sampling_length = seg_length / static_cast<F>(nb_sampling_edges);
        const F dl = seg_length / static_cast<F>(SAMPLING_BASE_N);
        CubicBezierMap2d<F> bezier(&m_control_points[6 * seg]);
        F cumul_l = F{0};
        for (std::size_t s = 0; s < nb_sampling_edges; s++)
        {
            float ratio_int = 0.f;
            const float ratio_frac = std::modf(static_cast<float>(cumul_l / dl), &ratio_int);
            const unsigned int idx = static_cast<unsigned int>(ratio_int);
            assert(idx < SAMPLING_BASE_N);
            // The vertex is between samples idx and (idx + 1)
            float time_ratio = ratio_frac;      // Linear model
            if (begin_max_rel_err[idx] > QUADRATIC_ARC_MODEL_RELATIVE_LENGTH_ERROR)
            {
                // Use the more precise quadratic model
                time_ratio = static_cast<float>(precise_time_ratio(begin_norm_v[idx], begin_norm_v[idx + 1], static_cast<F>(ratio_frac)));
            }
            assert(0.f <= time_ratio && time_ratio <= 1.f);
            result.vertices.emplace_back(bezier.at((1.f - time_ratio) * begin_sample_t[idx] + time_ratio * begin_sample_t[idx + 1]));
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

template <typename F, template<typename> typename P>
stdutils::stats::Result<F> path_normalized_uniformity_stats(const shapes::PointPath<P<F>>& pp)
{
    if (pp.vertices.empty())
        return stdutils::stats::Result<F>{};

    const auto nb_vertices = pp.vertices.size();
    const auto nb_edges = shapes::nb_edges(pp);
    std::vector<F> edge_lengths;
    edge_lengths.reserve(nb_edges);
    for (std::size_t v_idx = 0u; v_idx < nb_edges; v_idx++)
    {
        const auto p0 = pp.vertices[v_idx];
        const auto p1 = pp.vertices[(v_idx + 1) % nb_vertices];
        edge_lengths.emplace_back(shapes::norm(p1 - p0));
    }
    stdutils::stats::CumulSamples<F> stats;
    stats.add_samples(edge_lengths.cbegin(), edge_lengths.cend());
    return stats.get_result().normalize_to_mean();
}

} // namespace shapes
