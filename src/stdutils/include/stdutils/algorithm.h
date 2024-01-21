// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <algorithm>
#include <cassert>
#include <queue>
#include <stdexcept>
#include <type_traits>

namespace stdutils {

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


//
//
// Implementation
//
//


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

} // namespace stdutils
