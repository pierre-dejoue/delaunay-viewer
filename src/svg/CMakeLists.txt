#
# svg: Encapsulate third party to load/save SVG files
#

include(compiler_options)

# Third parties
include(bx)
include(simple-svg)

# ssvg_init: Wrapper for simple-svg's initialization, that explicitly depends on 'bx'.
# The compilation of 'bx' generates a lot of warnings, that is why it is isolated.

set(SSVG_INIT_SOURCES
    src/ssvg/ssvg_init.cpp
)

add_library(ssvg_init STATIC ${SSVG_INIT_SOURCES})

target_include_directories(ssvg_init
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ssvg/
)

target_link_libraries(ssvg_init
    PRIVATE
    bx
    simple-svg
)

# SVG module

set(LIB_SOURCES
    src/svg.cpp
)

add_library(svg STATIC ${LIB_SOURCES})

target_include_directories(svg
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_warnings(svg ON)

if(MSVC)
    # Correct definition of macro __cplusplus on Visual Studio
    # https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
    target_compile_options(ssvg_init PRIVATE "/Zc:__cplusplus")
    target_compile_options(svg PRIVATE "/Zc:__cplusplus")
endif()

target_link_libraries(svg
    PUBLIC
    shapes
    stdutils
    PRIVATE
    simple-svg
    ssvg_init
)

set_property(TARGET ssvg_init PROPERTY FOLDER "libraries")
set_property(TARGET svg PROPERTY FOLDER "libraries")
