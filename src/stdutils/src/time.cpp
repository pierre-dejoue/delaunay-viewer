// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License

// Prevent a warning on std::localtime with MSVC
#define _CRT_SECURE_NO_WARNINGS

#include <stdutils/time.h>

#include <stdutils/minmax.h>

#include <cassert>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace stdutils {

int tm_mon_from_abbrev(std::string_view Mmm)
{
    assert(Mmm.size() == 3);
    int tm_mon = -1;
    if      (Mmm == "Jan") { tm_mon = 0;  }
    else if (Mmm == "Feb") { tm_mon = 1;  }
    else if (Mmm == "Mar") { tm_mon = 2;  }
    else if (Mmm == "Apr") { tm_mon = 3;  }
    else if (Mmm == "May") { tm_mon = 4;  }
    else if (Mmm == "Jun") { tm_mon = 5;  }
    else if (Mmm == "Jul") { tm_mon = 6;  }
    else if (Mmm == "Aug") { tm_mon = 7;  }
    else if (Mmm == "Sep") { tm_mon = 8;  }
    else if (Mmm == "Oct") { tm_mon = 9;  }
    else if (Mmm == "Nov") { tm_mon = 10; }
    else if (Mmm == "Dec") { tm_mon = 11; }
    assert(tm_mon != -1);
    return stdutils::clamp<int>(tm_mon, 0, 11);
}

std::string_view abbrev_from_tm_mon(int tm_mon)
{
    assert(tm_mon < 12);    // Input month is in range 0 (JAN) to 11 (DEC)
    std::string_view out;
    switch (tm_mon)
    {
        case 0:
            out = "Jan";
            break;
        case 1:
            out = "Feb";
            break;
        case 2:
            out = "Mar";
            break;
        case 3:
            out = "Apr";
            break;
        case 4:
            out = "May";
            break;
        case 5:
            out = "Jun";
            break;
        case 6:
            out = "Jul";
            break;
        case 7:
            out = "Aug";
            break;
        case 8:
            out = "Sep";
            break;
        case 9:
            out = "Oct";
            break;
        case 10:
            out = "Nov";
            break;
        case 11:
            out = "Dec";
            break;
        default:
            assert(0);
            out = "Unk";
            break;
    }
    assert(out.size() == 3);
    return out;
}

std::string MonthYear::str() const
{
    std::stringstream out;
    out << *this;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const MonthYear& month_year)
{
    return out << abbrev_from_tm_mon(static_cast<int>(month_year.month_from_zero)) << '-' << (2000 + month_year.year_since_2000);
}

MonthYear get_build_date()
{
    const std::string build_date(__DATE__);         // E.g. "Mar  1 2025"
    assert(build_date.size() == 11);                //       012 45 789A

    const unsigned int tm_mon = static_cast<unsigned int>(tm_mon_from_abbrev(build_date.substr(0, 3)));
    const unsigned int yr =
        static_cast<unsigned int>(build_date[7]  - '0') * 1000 +
        static_cast<unsigned int>(build_date[8]  - '0') * 100  +
        static_cast<unsigned int>(build_date[9]  - '0') * 10   +
        static_cast<unsigned int>(build_date[10] - '0') * 1;
    assert(yr >= 2000);

    const MonthYear result(tm_mon + 1, yr - 2000u);
    assert(result.is_valid());
    return result;
}

MonthYear current_local_date()
{
    const auto now = std::time(nullptr);
    const auto* local_now = std::localtime(&now);
    MonthYear month_year;
    if (local_now != nullptr)
    {
        assert(local_now->tm_year >= 100);
        month_year.year_since_2000 = static_cast<unsigned int>(local_now->tm_year - 100);
        month_year.month_from_zero = static_cast<unsigned int>(local_now->tm_mon);
    }
    assert(month_year.is_valid());
    return month_year;
}

std::string current_local_date_and_time()
{
    const auto now = std::time(nullptr);
    const auto* local_now = std::localtime(&now);
    if (local_now == nullptr) { return "No date"; }
    std::stringstream out;
    out << std::put_time(local_now, "%b %d %Y %H:%M:%S");
    return out.str();
}

std::string_view build_date()
{
    return __DATE__;
}

std::string_view build_date_and_time()
{
    return __DATE__ " " __TIME__;
}

} // namespace stdutils
