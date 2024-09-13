// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

int __macos__get_local_app_data_path(char* buffer, unsigned int max_sz);

#if defined(__cplusplus)
}
#endif
