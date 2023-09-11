#pragma once

#include <shapes/point.h>
#include <shapes/path.h>


namespace shapes
{

/**
 * Uniform sampling generic interface
 */
template <typename F, template<typename> typename P>
class UniformSamplingInterface
{
public:
    UniformSamplingInterface() = default;
    virtual ~UniformSamplingInterface() = default;
    virtual F max_segment_length() const = 0;
    virtual PointPath<P<F>> sample(F max_sampling_length) const = 0;
};

template <typename F>
using UniformSamplingInterface2d = UniformSamplingInterface<F, Point2d>;

} // namespace shapes
