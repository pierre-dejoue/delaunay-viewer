// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <cassert>
#include <string>
#include <string_view>

/**
 * Utility functions related to time (current time, compilation time)
 *
 * Introduce a custom MonthYear struct as a convenient way to count months since 2000.
 */
namespace stdutils {

/**
 * Back and forth conversions of:
 *  - The three-letter English abbreviated month name, Mmm, one of Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec.
 *  - A month number from 0 (January) to 11 (December) (Cf. std::tm->tm_mon)
 */
int tm_mon_from_abbrev(std::string_view Mmm);
std::string_view abbrev_from_tm_mon(int tm_mon);

/**
 * Date limited to the year and the month
 */
struct MonthYear
{
    /**
     * MonthYear default ctor. Corresponds to January 2000.
     */
    constexpr MonthYear()
        : month_from_zero(0)
        , year_since_2000(0)
    { }

    /**
     * MonthYear human-readable ctor. For example, MonthYear(11, 24) corresponds to NOV-2024.
     */
    constexpr MonthYear(unsigned int month, unsigned int year_since_2000)
        : month_from_zero(month - 1)
        , year_since_2000(year_since_2000)
    {
        assert(is_valid());
    }

    unsigned int month_from_zero;            // 0 (January) to 11 (December)
    unsigned int year_since_2000;            // e.g. 24 for 2024

    constexpr unsigned int months() const { return 12 * year_since_2000 + month_from_zero; }

    constexpr bool is_valid() const { return month_from_zero < 12; }

    std::string str() const;
};

std::ostream& operator<<(std::ostream& out, const MonthYear& month_year);

MonthYear get_build_date();

/**
 * Current local date as a MonthYear
 */
MonthYear current_local_date();

/**
 * Current local date and time as a string
 */
std::string current_local_date_and_time();

/**
 * Compilation date and time
 */
std::string_view build_date();
std::string_view build_date_and_time();

} // namespace stdutils
