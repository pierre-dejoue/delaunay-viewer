cmake_minimum_required(VERSION 3.20)

project(delaunay_viewer
    DESCRIPTION "Delaunay Viewer"
    VERSION 0.1.0
)

option(DELAUNAY_VIEWER_BUILD_POLY2TRI "Build with the library poly2tri" ON)
option(DELAUNAY_VIEWER_BUILD_CDT "Build with the library CDT" ON)
option(DELAUNAY_VIEWER_BUILD_UTESTS "Build unit tests" OFF)
option(DELAUNAY_VIEWER_IMGUI_DEMO "Show ImGUI demo window" OFF)

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

