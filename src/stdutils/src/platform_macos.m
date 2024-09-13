// Copyright (c) 2024 Pierre DEJOUE
// This code is distributed under the terms of the MIT License

// Platform specific source code for macOS, in Objective C

#import <Cocoa/Cocoa.h>

int __macos__get_local_app_data_path(char* buffer, unsigned int max_sz)
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *applicationSupportDirectory = [paths firstObject];
    BOOL success = [applicationSupportDirectory getCString:buffer maxLength:max_sz encoding:NSUTF8StringEncoding];
    return (success == YES) ? 1 : 0;
}
