// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <stdutils/macros.h>
#include <stdutils/span.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <stdexcept>

namespace stdutils {

/**
 * A buffer which size is fixed and set at runtime
 *
 * When calling FixedBuffer(n):
 *  - If T is a class type the buffer is initialized with the default constructor.
 *  - Otherwise, the buffer is uninitialized.
 *
 * That buffer is mostly intended for POD types, to benefit from the fact that the memory is allocated and not initialized.
 * It will work with class types as well, but with a penalty cost on some of the constructors (see TODO notes)
 */
template <typename T>
class FixedBuffer {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;

    // Trait
    struct memory_is_allocated_and_uninitialized
        : std::is_trivially_default_constructible<T>
    { };

    FixedBuffer() noexcept;
    explicit FixedBuffer(std::size_t n);
    FixedBuffer(std::size_t n, const T& v);                 // Force initialization of the memory

    // Non-copyable. Copy of the buffer is meant to be explicit.
    FixedBuffer(const FixedBuffer&) = delete;
    FixedBuffer& operator=(const FixedBuffer& ) = delete;

    // Moveable
    FixedBuffer(FixedBuffer&& other) noexcept;
    FixedBuffer& operator=(FixedBuffer&& other) noexcept;

    std::size_t size() const noexcept;
    bool empty() const noexcept;

    void init(const T& v);

    pointer data() noexcept;
    const_pointer data() const noexcept;

    Span<T> span() noexcept;
    Span<const T> cspan() const noexcept;

    // For compatibility with other containers. Does nothing.
    void clear() noexcept;

    // Truncate to a smaller size. The same memory buffer is kept and no reallocation occurs.
    void truncate(std::size_t n);

    // Comparison
    bool operator==(const FixedBuffer<T>& other) const;
    bool operator!=(const FixedBuffer<T>& other) const;

private:
    std::unique_ptr<T[]>    m_buffer;
    std::size_t             m_size;
};

// Explicit copy of one fixed buffer into another. The sizes of both buffers MUST match.
template <typename T>
void copy(FixedBuffer<T>& dest, const FixedBuffer<T>& src);

// A raw memory buffer
using FixedByteBuffer = FixedBuffer<std::byte>;


//
//
// Implementation
//
//


template <typename T>
FixedBuffer<T>::FixedBuffer() noexcept
    : m_buffer(std::unique_ptr<T[]>())
    , m_size(0)
{ }

template <typename T>
FixedBuffer<T>::FixedBuffer(std::size_t n)
    : m_buffer(std::unique_ptr<T[]>(new T[n]))
    , m_size(n)
{ }

// TODO: For non-POD types we get a double initialization cost
template <typename T>
FixedBuffer<T>::FixedBuffer(std::size_t n, const T& v)
    : m_buffer(std::unique_ptr<T[]>(new T[n]))
    , m_size(n)
{
    init(v);
}

template <typename T>
FixedBuffer<T>::FixedBuffer(FixedBuffer&& other) noexcept
    : m_buffer(std::move(other.m_buffer))
    , m_size(other.m_size)
{
    other.m_size = 0;
    assert(!other.m_buffer);
}

template <typename T>
FixedBuffer<T>& FixedBuffer<T>::operator=(FixedBuffer&& other) noexcept
{
    m_buffer = std::move(other.m_buffer);
    m_size = other.m_size;
    other.m_size = 0;
    assert(!other.m_buffer);
    return *this;
}

template <typename T>
std::size_t FixedBuffer<T>::size() const noexcept
{
    return m_size;
}

template <typename T>
bool FixedBuffer<T>::empty() const noexcept
{
    return m_size == 0;
}

template <typename T>
void FixedBuffer<T>::init(const T& v)
{
    std::fill_n(m_buffer.get(), m_size, v);
}

template <typename T>
typename FixedBuffer<T>::pointer FixedBuffer<T>::data() noexcept
{
    return m_buffer.get();
}

template <typename T>
typename FixedBuffer<T>::const_pointer FixedBuffer<T>::data() const noexcept
{
    return m_buffer.get();
}

template <typename T>
Span<T> FixedBuffer<T>::span() noexcept
{
    T* ptr = m_buffer.get();
    return ptr ? Span<T>(ptr, m_size) : Span<T>();
}

template <typename T>
Span<const T> FixedBuffer<T>::cspan() const noexcept
{
    const T* ptr = m_buffer.get();
    return ptr ? Span<const T>(ptr, m_size) : Span<const T>();
}

template <typename T>
void FixedBuffer<T>::clear() noexcept
{
    // For compatibility with other containers. Does nothing.
}

template <typename T>
void FixedBuffer<T>::truncate(std::size_t n)
{
    if (n > m_size)
    {
        throw std::invalid_argument("Truncation size greater than the current size");
    }
    m_size = n;
}

template <typename T>
bool FixedBuffer<T>::operator==(const FixedBuffer<T>& other) const
{
    const Span<const T> this_span = cspan();
    const Span<const T> other_span = other.cspan();
    return std::equal(this_span.begin(), this_span.end(), other_span.begin(), other_span.end());
}

template <typename T>
bool FixedBuffer<T>::operator!=(const FixedBuffer<T>& other) const
{
    return !(*this == other);
}

template <typename T>
void copy(FixedBuffer<T>& dest, const FixedBuffer<T>& src)
{
    if (dest.size() != src.size())
    {
        throw std::invalid_argument("FixedBuffer size mismatch");
    }
    if (src.size() > 0)
    {
        assert(src.data());
        assert(dest.data());
        IGNORE_RETURN std::copy_n<const T*, std::size_t, T*>(src.data(), src.size(), dest.data());
    }
}

} // namespace stdutils
