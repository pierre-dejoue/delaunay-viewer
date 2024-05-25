// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#include <algorithm>

struct Parameter
{
    template <typename T>
    struct Limits
    {
        T def;
        T min;
        T max;

        void clamp(T& v) const { v = std::clamp(v, min, max); }
    };
    static constexpr Limits<bool> limits_true { true, false, true };
    static constexpr Limits<bool> limits_false { false, false, true };
};
