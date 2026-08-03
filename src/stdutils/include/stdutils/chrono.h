// Copyright (c) 2021 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>
#include <type_traits>

namespace stdutils {
namespace chrono {

/**
 * Usage: For example, to measure a duration in milliseconds:
 *
 *  std::chrono::duration<float, std::milli> duration;
 *  {
 *      stdutils::chrono::DurationMeas meas(duration);
 *      // Do something
 *  }
 *  const float duration_ms = duration.count();
 */
template <typename _Rep, typename _Period, typename _Clock = std::chrono::steady_clock>
class DurationMeas
{
public:
    using clock_t = _Clock;
    using duration_t = std::chrono::duration<_Rep, _Period>;
    using time_point_t = std::chrono::time_point<_Clock>;

    explicit DurationMeas(duration_t& out_duration)
        : m_out_duration(out_duration)
        , m_start(clock_t::now())
    { }

    ~DurationMeas()
    {
        const time_point_t end = clock_t::now();
        m_out_duration = std::chrono::duration_cast<duration_t>(end - m_start);
    }

private:
    duration_t&  m_out_duration;
    time_point_t m_start;
};

template <typename _Duration, typename _Clock = std::chrono::steady_clock>
class Timeout
{
public:
    using clock_t = _Clock;
    using duration_t = _Duration;
    using time_point_t = std::chrono::time_point<_Clock, _Duration>;

    explicit Timeout(duration_t duration)
        : m_timeout_end(std::chrono::time_point_cast<duration_t>(clock_t::now()))
    {
        m_timeout_end += duration;
    }

    bool has_expired() const
    {
        return clock_t::now() > m_timeout_end;
    }
private:
    time_point_t m_timeout_end;
};

template <typename _Rep, typename _Period, typename _Clock = std::chrono::steady_clock>
class PeriodMeas
{
public:
    using rep_t = _Rep;
    using clock_t = _Clock;
    using duration_t = std::chrono::duration<_Rep, _Period>;
    using time_point_t = std::chrono::time_point<_Clock>;

    PeriodMeas()
        : m_start(clock_t::now())
        , m_period()
    {}

    PeriodMeas& measure()
    {
        const time_point_t now = clock_t::now();
        m_period = std::chrono::duration_cast<duration_t>(now - m_start);
        m_start = now;
        return *this;
    }

    const time_point_t& start() const { return m_start; }
    rep_t period() const { return m_period.count(); }

private:
    time_point_t m_start;
    duration_t   m_period;
};

using PeriodMeasSeconds = PeriodMeas<float, std::ratio<1>>;

/**
 * Reference time measured from the specified origin time point
 */
template <typename _Clock = std::chrono::steady_clock>
class Now
{
public:
    using clock_t = _Clock;
    using duration_t = typename _Clock::duration;
    using time_point_t = std::chrono::time_point<_Clock>;

    Now()
        : m_now()
    { }

    Now(const time_point_t& origin)
        : m_now(clock_t::now() - origin)
    { }

    template <typename _Duration>
    _Duration time() const
    {
        return std::chrono::duration_cast<_Duration>(m_now);
    }

    template <typename _Duration>
    std::uint64_t time_integral() const
    {
        static_assert(std::is_integral_v<typename _Duration::rep>);
        using _Period = typename _Duration::period;
        return std::chrono::duration_cast<std::chrono::duration<std::uint64_t, _Period>>(m_now).count();
    }

    // Return the fractional part of time now, for the given duration period.
    template <typename F, typename _Period>
    F time_fractional() const
    {
        static_assert(std::is_floating_point_v<F>);
        using IntPeriod = std::chrono::duration<std::uint64_t, _Period>;
        const duration_t elapsed = m_now;
        const duration_t elapsed_floor = std::chrono::duration_cast<duration_t>(std::chrono::floor<IntPeriod>(m_now));
        return std::chrono::duration_cast<std::chrono::duration<F, _Period>>(elapsed - elapsed_floor).count();
    }

    template <typename F>
    F fractional_seconds() const
    {
        return time_fractional<F, std::ratio<1>>();
    }

private:
    duration_t m_now;
};

/**
 * TimeServer: Measure the time since an origin set at the construction of the object
 *
 * A typical usage will be to measure the time since starting the application
 */
template <typename _Clock = std::chrono::steady_clock>
class TimeServer
{
public:
    using clock_t = _Clock;
    using time_point_t = std::chrono::time_point<_Clock>;

    TimeServer()
        : m_start(clock_t::now())
    { }

    Now<clock_t> now() const
    {
        return Now<clock_t>(m_start);
    }

private:
    time_point_t m_start;
};

} // namespace chrono
} // namespace stdutils
