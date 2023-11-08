#pragma once

#if defined(__GNUC__)
#pragma GCC system_header
#endif

#include <portable-file-dialogs.h>

// Prevent namespace pollution
#ifdef _WIN32
#undef min
#undef max
#endif
