if(TARGET simple-svg)
    return()
endif()

message(STATUS "Third-party: simple-svg")

# This library depends on third-party "bx". Please include(bx) first.

include(FetchContent)
FetchContent_Declare(
    simple-svg
    #GIT_REPOSITORY https://github.com/jdryg/simple-svg.git
    #GIT_TAG 279c682320a882661f05e30e6e6e949a61a31293        # Nov 30, 2021
    # Fork:
    GIT_REPOSITORY https://github.com/pierre-dejoue/simple-svg.git
    GIT_TAG eeb4ff7f35e3479c1acf59d5668dfe376f3e937d
)
FetchContent_Populate(simple-svg)

set(SIMPLE_SVG_SOURCES
    ${simple-svg_SOURCE_DIR}/src/ssvg.cpp
    ${simple-svg_SOURCE_DIR}/src/ssvg_builder.cpp
    ${simple-svg_SOURCE_DIR}/src/ssvg_parser.cpp
    ${simple-svg_SOURCE_DIR}/src/ssvg_writer.cpp
)

set(SIMPLE_SVG_HEADERS
    ${simple-svg_SOURCE_DIR}/include/ssvg/ssvg.h
)

add_library(simple-svg STATIC ${SIMPLE_SVG_SOURCES} ${SIMPLE_SVG_HEADERS})

target_include_directories(simple-svg
    PUBLIC
    ${simple-svg_SOURCE_DIR}/include
)

if(MSVC)
    # Correct definition of macro __cplusplus on Visual Studio
    # https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
    target_compile_options(simple-svg PRIVATE "/Zc:__cplusplus")
endif()

target_link_libraries(simple-svg
    PRIVATE
    bx
)

set_property(TARGET simple-svg PROPERTY FOLDER "third_parties")

