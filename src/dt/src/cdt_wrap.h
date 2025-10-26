// Copyright (c) 2023 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244 )               // Warning C4244: conversion with possible loss of data
#pragma warning( disable : 4267 )               // Warning C4267: conversion with possible loss of data
#pragma warning( disable : 4305 )               // Warning C4701: truncation from 'double' to ...
#pragma warning( disable : 4701 )               // Warning C4701: potentially uninitialized local variable
#pragma warning( disable : 6001 )               // Warning C6001: using uninitialized memory
#endif

#if defined(__GNUC__)
#pragma GCC system_header
#pragma GCC diagnostic ignored "-Wnull-dereference"
#endif

#include <CDT.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif
