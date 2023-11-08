// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

namespace stdutils
{

// Helper type for visitors
template<class... Ts>
struct Overloaded : Ts... { using Ts::operator()...; };

// Explicit deduction guide (not needed as of C++20)
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

}
