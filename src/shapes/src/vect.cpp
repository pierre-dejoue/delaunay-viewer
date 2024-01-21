// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#include <shapes/vect.h>

#include <stdutils/io.h>

#include <iomanip>

namespace shapes {

//
// IO
//
template <typename F>
std::ostream& operator<<(std::ostream& out, const Vect2d<F>& v)
{
    const auto initial_fp_digits = stdutils::io::accurate_fp_precision<F>(out);
    out << "(" << v.x << ", " << v.y << ")";
    return out << std::setprecision(initial_fp_digits);
}

template <typename F>
std::ostream& operator<<(std::ostream& out, const Vect3d<F>& v)
{
    const auto initial_fp_digits = stdutils::io::accurate_fp_precision<F>(out);
    out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return out << std::setprecision(initial_fp_digits);
}

// Explicit template instantiations
template std::ostream& operator<<(std::ostream& out, const Vect2d<float>& v);
template std::ostream& operator<<(std::ostream& out, const Vect2d<double>& v);
template std::ostream& operator<<(std::ostream& out, const Vect3d<float>& v);
template std::ostream& operator<<(std::ostream& out, const Vect3d<double>& v);

} // namespace shapes
