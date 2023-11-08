// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 6001 )               // Warning C6001: Using uninitialized memory
#pragma warning( disable : 4701 )               // Warning C4701: potentially uninitialized local variable
#endif

#if defined(__GNUC__)
#pragma GCC system_header
#endif

#include <CDT.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif
