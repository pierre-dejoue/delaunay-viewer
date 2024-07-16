// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/enum.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <queue>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace stdutils {

// Min/max updates
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

// Like std::clamp but with an output boolean flag 'clamped'
template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi, bool& clamped) { assert(lo <= hi); const T& r = std::clamp(v, lo, hi); clamped = (r != v); return r; }
template <class T, class Compare>
constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp, bool& clamped) { assert(!comp(hi, lo)); const T& r = std::clamp(v, lo, hi, comp); clamped = (r != v); return r; }

// Clamp enumeration values assuming the enumeration is a range starting with value 0 and ending with the special value _ENUM_SIZE_
// NB: Cannot return const E& because the min/max clamp values are local to the definition
template <class E>
constexpr E clamp_enum(const E& v, bool& clamped) { static_assert(std::is_enum_v<E>); return clamp(v, enum_first_value<E>(), enum_last_value<E>(), clamped); }

// Sort three elements
template <typename T>
void three_sort(T* arr);
template <typename T, typename Compare>
void three_sort(T* arr, Compare comp);

// Equivalent to std::erase/erase_if in C++20
constexpr bool SHRINK_TO_FIT = true;
template <class Container, class U>
std::size_t erase(Container& c, const U& value, bool shrink_to_fit = false);
template <class Container, class Pred>
std::size_t erase_if(Container& c, Pred pred, bool shrink_to_fit = false);

// Find by index in a random access container
template <typename C, typename I>
I index_find(const C& container, I begin, I end, const typename C::value_type& value);
template <typename C, typename I, typename P>
I index_find_if(const C& container, I begin, I end, P predicate);

// Utility for containers that pops: Move and pop the front/back element in one call.
//   Use pop_back  with: list, deque, string, vector
//   Use pop_front with: list, forward_list, deque
//   Use pop       with: queue, stack, priority_queue
template <typename T, template <typename...> class C>
T pop_back(C<T>& container);
template <typename T, template <typename...> class C>
T pop_front(C<T>& container);
template <typename T, template <typename...> class C, std::enable_if_t<!std::is_same_v<C<T>, std::queue<T>>, bool> = true>
T pop(C<T>& container);
template <typename T>
T pop(std::queue<T>& queue);

// Clear using pop. Use with: queue, stack, priority_queue
template <typename T, template <typename...> class C>
void clear_using_pop(C<T>& container);

// Stable merge (based on std::merge)
template <typename T, template <typename...> class C, typename Compare>
void merge(C<T>& dst, const C<T>& src, Compare comp);

template <typename Func>
void repeat_n_times(std::size_t n, Func f);


//
//
// Implementation
//
//


template <typename T>
void three_sort(T* arr)
{
    assert(arr != nullptr);
    bool need_last_comp = false;
    if (                  !(arr[0] < arr[1])) { std::swap(arr[0], arr[1]); }
    if (                  !(arr[1] < arr[2])) { std::swap(arr[1], arr[2]); need_last_comp = true; }
    if (need_last_comp && !(arr[0] < arr[1])) { std::swap(arr[0], arr[1]); }
}

template <typename T, typename Compare>
void three_sort(T* arr, Compare comp)
{
    assert(arr != nullptr);
    bool need_last_comp = false;
    if (                  !comp(arr[0], arr[1])) { std::swap(arr[0], arr[1]); }
    if (                  !comp(arr[1], arr[2])) { std::swap(arr[1], arr[2]); need_last_comp = true; }
    if (need_last_comp && !comp(arr[0], arr[1])) { std::swap(arr[0], arr[1]); }
}

template <class Container, class U>
std::size_t erase(Container& c, const U& value, bool shrink_to_fit)
{
    const auto it = std::remove(c.begin(), c.end(), value);
    const auto erased = c.end() - it;
    c.erase(it, c.end());
    if (shrink_to_fit) { c.shrink_to_fit(); }
    assert(erased >= 0);
    return static_cast<std::size_t>(erased);
}

template <class Container, class Pred>
std::size_t erase_if(Container& c, Pred pred, bool shrink_to_fit)
{
    const auto it = std::remove_if(c.begin(), c.end(), pred);
    const auto erased = c.end() - it;
    c.erase(it, c.end());
    if (shrink_to_fit) { c.shrink_to_fit(); }
    assert(erased >= 0);
    return static_cast<std::size_t>(erased);
}

template <typename C, typename I>
I index_find(const C& container, I begin, I end, const typename C::value_type& value)
{
    const auto u_begin = static_cast<std::size_t>(begin);
    const auto u_end = static_cast<std::size_t>(end);
    assert(u_begin <= u_end);
    assert(u_end <= container.size());
    for (auto idx = u_begin; idx < u_end; ++idx)
    {
        if (container[idx] == value) { return static_cast<I>(idx); }
    }
    return static_cast<I>(end);
}

template <typename C, typename I, typename P>
I index_find_if(const C& container, I begin, I end, P predicate)
{
    const auto u_begin = static_cast<std::size_t>(begin);
    const auto u_end = static_cast<std::size_t>(end);
    assert(u_begin <= u_end);
    assert(u_end <= container.size());
    for (auto idx = u_begin; idx < u_end; ++idx)
    {
        if (predicate(container[idx])) { return static_cast<I>(idx); }
    }
    return static_cast<I>(u_end);
}

template <typename T, template <typename...> class C>
T pop_back(C<T>& container)
{
    if (container.empty())
        throw std::invalid_argument("Cannot pop from an empty container");
    T elt = std::move(container.back());
    container.pop_back();
    return elt;
}

template <typename T, template <typename...> class C>
T pop_front(C<T>& container)
{
    if (container.empty())
        throw std::invalid_argument("Cannot pop from an empty container");
    T elt = std::move(container.front());
    container.pop_front();
    return elt;
}

template <typename T, template <typename...> class C, std::enable_if_t<!std::is_same_v<C<T>, std::queue<T>>, bool>>
T pop(C<T>& container)
{
    if (container.empty())
        throw std::invalid_argument("Cannot pop from an empty container");
    T elt = std::move(container.top());
    container.pop();
    return elt;
}

template <typename T>
T pop(std::queue<T>& queue)
{
    if (queue.empty())
        throw std::invalid_argument("Cannot pop from an empty queue");
    T elt = std::move(queue.front());
    queue.pop();
    return elt;
}

template <typename T, template <typename...> class C>
void clear_using_pop(C<T>& container)
{
    while (!container.empty()) { container.pop(); }
}

template <typename T, template <typename...> class C, typename Compare>
void merge(C<T>& dst, const C<T>& src, Compare comp)
{
    C<T> tmp;
    tmp.reserve(dst.size() + src.size());
    std::merge(dst.cbegin(), dst.cend(), src.cbegin(), src.cend(), std::back_insert_iterator(tmp), comp);
    std::swap(tmp, dst);
}

template <typename Func>
void repeat_n_times(std::size_t n, Func f)
{
    while (n--) { f(); }
}

} // namespace stdutils
