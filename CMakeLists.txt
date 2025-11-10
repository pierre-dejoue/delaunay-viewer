cmake_minimum_required(VERSION 3.28)

# - On XCODE 14, 15 and 16 the range of supported deployment targets begins with macOS 10.13 High Sierra
#   Reference: https://developer.apple.com/support/xcode/
# - Support for std::filesystem::path however is only available starting from macOS 10.15 Catalina
# - With the arm64 architecture, the deployment target will be automagically set to 11.0, the first macOS version to support the M1 processor
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment target" FORCE)

project(delaunay_viewer
    DESCRIPTION "Delaunay Viewer"
    VERSION 0.1.1
)

set(DELAUNAY_VIEWER_OFFICIAL_RELEASE OFF)

set(THIS_PROJECT_OWNER "Pierre DEJOUE")
set(THIS_PROJECT_YEAR_RANGE "2023-2025")
set(THIS_PROJECT_SHORT_LICENSE "MIT license")
set(THIS_PROJECT_SHORT_COPYRIGHT "Copyright (c) ${THIS_PROJECT_YEAR_RANGE} ${THIS_PROJECT_OWNER}")
set(THIS_PROJECT_COPYRIGHT "${THIS_PROJECT_SHORT_COPYRIGHT}. ${THIS_PROJECT_SHORT_LICENSE}.")
set(THIS_PROJECT_WEBSITE "https://github.com/pierre-dejoue/delaunay-viewer")

option(DELAUNAY_VIEWER_BUILD_POLY2TRI "Build with the library poly2tri" ON)
option(DELAUNAY_VIEWER_BUILD_CDT "Build with the library CDT" ON)
option(DELAUNAY_VIEWER_BUILD_TRIANGLE "Build with Shewchuk's Triangle library" ON)
option(DELAUNAY_VIEWER_BUILD_UTESTS "Build unit tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/third_party/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/config/")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Stdutils
add_subdirectory(src/stdutils)

# Linear algebra
add_subdirectory(src/lin)

# Graph, geometrical shapes
add_subdirectory(src/graphs)
add_subdirectory(src/shapes)

# Delaunay libs
add_subdirectory(src/dt)

# SVG
add_subdirectory(src/svg)

# GUI
add_subdirectory(src/gui)

# Tests
if(DELAUNAY_VIEWER_BUILD_UTESTS)
    add_subdirectory(src/tests/stdutils)
    add_subdirectory(src/tests/lin)
    add_subdirectory(src/tests/shapes)
endif()

set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)
include(CPack)

