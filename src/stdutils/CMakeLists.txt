#
# General utilities common to the library and the applications
#

include(compiler_options)

set(LIB_SOURCES
    src/io.cpp
    src/platform.cpp
    src/string.cpp
    src/time.cpp
)

if(APPLE)
    list(APPEND LIB_SOURCES src/platform_macos.m)
endif()

file(GLOB LIB_HEADERS include/stdutils/*.h src/*.h)

add_library(stdutils STATIC ${LIB_SOURCES} ${LIB_HEADERS})

target_include_directories(stdutils
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_warnings(stdutils ON)

if(APPLE)
    target_link_libraries(stdutils
        PRIVATE
        "-framework Cocoa"
    )
endif()

if(MSVC)
    # Correct definition of macro __cplusplus on Visual Studio
    # https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
    target_compile_options(stdutils PRIVATE "/Zc:__cplusplus")
endif()

set_property(TARGET stdutils PROPERTY FOLDER "libraries")
