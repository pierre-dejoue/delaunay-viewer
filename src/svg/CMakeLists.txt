#
# svg: Encapsulate third party to load/save SVG files
#

include(compiler_options)

# Third parties
include(bx)
include(simple-svg)

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
    target_compile_options(svg PRIVATE "/Zc:__cplusplus")
endif()

target_link_libraries(svg
    PUBLIC
    shapes
    stdutils
    PRIVATE
    simple-svg
)

set_property(TARGET svg PROPERTY FOLDER "libraries")

