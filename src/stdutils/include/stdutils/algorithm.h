#pragma once

#include <algorithm>
#include <cassert>

namespace stdutils
{

template <class T>
constexpr void max_update(T& to, const T& from) { to = std::max(to, from); }
template< class T, class Compare>
constexpr void max_update(T& to, const T& from, Compare comp) { to = std::max(to, from, comp); }

template <class T>
constexpr void min_update(T& to, const T& from) { to = std::min(to, from); }
template< class T, class Compare>
constexpr void min_update(T& to, const T& from, Compare comp) { to = std::min(to, from, comp); }

template <class T>
constexpr void minmax_update(std::pair<T, T>& to, const T& from) { assert(to.first <= to.second); if (from < to.first) { to.first = from; } else if (to.second < from) { to.second = from; } }
template <class T, class Compare>
constexpr void minmax_update(std::pair<T, T>& to, const T& from, Compare comp) { assert(!comp(to.second, to.first)); if (comp(from, to.first)) { to.first = from; } else if (comp(to.second, from)) { to.second = from; } }

} // namespace stdutils
