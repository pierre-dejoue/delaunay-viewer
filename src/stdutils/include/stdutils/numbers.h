#pragma once

namespace stdutils
{
namespace numbers
{

template <typename F>
inline constexpr F e_v = F{};   // Invalid
template <typename F>
inline constexpr F pi_v = F{};   // Invalid

template <>
inline constexpr double e_v<double> = 2.718281828459045;
template <>
inline constexpr double pi_v<double> = 3.141592653589793;

template <>
inline constexpr float e_v<float> = static_cast<float>(e_v<double>);
template <>
inline constexpr float pi_v<float> = static_cast<float>(pi_v<double>);

inline constexpr double e = e_v<double>;
inline constexpr double pi = pi_v<double>;

} // namespace numbers
} // namespace stdutils
