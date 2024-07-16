// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace stdutils {

/**
 * A locked buffer stores data and can be locked.
 *
 *  - When LOCKED, the buffer is read-only.
 *  - When UNLOCKED, the buffer is freely accessible.
 *
 * An index is managed by the user. The index is ALIGNED with the buffer when it points to the past-the-end element.
 * Alignment can be checked manually thanks to method index_is_aligned(), and it is a requirement to lock the buffer.
 *
 * See unit tests for a use case.
 */
template <typename T, template <typename...> class C = std::vector>
class LockedBuffer
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using value_container = C<T>;
    using size_type = std::size_t;

    LockedBuffer() noexcept;
    LockedBuffer(value_container&& container) noexcept;
    LockedBuffer(const LockedBuffer&) = delete;
    LockedBuffer& operator=(const LockedBuffer&) = delete;
    LockedBuffer(LockedBuffer&&) noexcept = default;
    LockedBuffer& operator=(LockedBuffer&&) noexcept = default;

    // Buffer accessors
    const_pointer data() const noexcept;
    size_type size() const noexcept;
    value_container& buffer();                          // Throws if the buffer is LOCKED

    // Index modifiers
    void index_reset() noexcept;
    void consume(size_type count);                      // Throws if the buffer's size is exceeded
    size_type consumed() const noexcept;

    // Index alignment
    bool index_is_aligned() const noexcept;             // Return true if the index points to the past-the-end element

    // Locking
    void lock();                                        // Throws if the buffer is already LOCKED or if the index is misaligned
    void unlock();                                      // Throws if the buffer is already UNLOCKED
    bool is_locked() const noexcept;
    bool is_unlocked() const noexcept;

    void clear() noexcept;                              // Clear the buffer and unlock it, reset the index (equivalent to a reset of the object)

private:
    value_container m_buffer;
    size_type       m_index;
    bool            m_locked;
};


//
//
// Implementation
//
//


template <typename T, template <typename...> class C>
LockedBuffer<T, C>::LockedBuffer() noexcept
    : m_buffer()
    , m_index{0u}
    , m_locked{false}
{ }

template <typename T, template <typename...> class C>
LockedBuffer<T, C>::LockedBuffer(value_container&& container) noexcept
    : m_buffer(std::move(container))
    , m_index{0u}
    , m_locked{false}
{ }

template <typename T, template <typename...> class C>
const T* LockedBuffer<T, C>::data() const noexcept
{
    return m_buffer.data();
}

template <typename T, template <typename...> class C>
typename LockedBuffer<T, C>::size_type LockedBuffer<T, C>::size() const noexcept
{
    return m_buffer.size();
}

template <typename T, template <typename...> class C>
typename LockedBuffer<T, C>::value_container& LockedBuffer<T, C>::buffer()
{
    if (m_locked)
    {
        throw std::runtime_error("buffer is locked");
    }
    return m_buffer;
}

template <typename T, template <typename...> class C>
void LockedBuffer<T, C>::index_reset() noexcept
{
    m_index = 0;
}

template <typename T, template <typename...> class C>
void LockedBuffer<T, C>::consume(size_type count)
{
    if (m_index + count > m_buffer.size())
    {
        throw std::runtime_error("buffer size exceeded");
    }
    m_index += count;
}

template <typename T, template <typename...> class C>
typename LockedBuffer<T, C>::size_type LockedBuffer<T, C>::consumed() const noexcept
{
    return m_index;
}

template <typename T, template <typename...> class C>
bool LockedBuffer<T, C>::index_is_aligned() const noexcept
{
    return m_index == m_buffer.size();
}

template <typename T, template <typename...> class C>
void LockedBuffer<T, C>::lock()
{
    if (m_locked)
    {
        throw std::runtime_error("buffer is already locked");
    }
    if (m_index != m_buffer.size())
    {
        throw std::runtime_error("buffer cannot be locked because the index is misaligned");
    }
    m_locked = true;
}

template <typename T, template <typename...> class C>
void LockedBuffer<T, C>::unlock()
{
    if (!m_locked)
    {
        throw std::runtime_error("buffer is already unlocked");
    }
    m_locked = false;
}

template <typename T, template <typename...> class C>
bool LockedBuffer<T, C>::is_locked() const noexcept
{
    return m_locked;
}

template <typename T, template <typename...> class C>
bool LockedBuffer<T, C>::is_unlocked() const noexcept
{
    return !m_locked;
}

template <typename T, template <typename...> class C>
void LockedBuffer<T, C>::clear() noexcept
{
    m_locked = false;
    m_buffer.clear();
    m_index = 0;
}

} // namespace stdutils
