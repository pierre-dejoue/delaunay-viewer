if(TARGET simpler-svg)
    return()
endif()

message(STATUS "Third-party: simpler-svg")

include(FetchContent)
FetchContent_Populate(
    simpler-svg
    QUIET
    GIT_REPOSITORY https://github.com/pierre-dejoue/simpler-svg.git
    GIT_TAG 4082d4bcf0c4b47884fb0329f2a99fc6a83f1e30        #  0.4 (Sept 2026)
)

set(SIMPLER_SVG_SOURCES
    ${simpler-svg_SOURCE_DIR}/ssvg/src/ssvg.cpp
    ${simpler-svg_SOURCE_DIR}/ssvg/src/ssvg_builder.cpp
    ${simpler-svg_SOURCE_DIR}/ssvg/src/ssvg_parser.cpp
    ${simpler-svg_SOURCE_DIR}/ssvg/src/ssvg_writer.cpp
)

set(SIMPLER_SVG_HEADERS
    ${simpler-svg_SOURCE_DIR}/ssvg/include/ssvg/ssvg.h
)

add_library(ssvg STATIC ${SIMPLER_SVG_SOURCES} ${SIMPLER_SVG_HEADERS})

target_include_directories(ssvg
    PUBLIC
    ${simpler-svg_SOURCE_DIR}/ssvg/include
)

if(MSVC)
    # Correct definition of macro __cplusplus on Visual Studio
    # https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
    target_compile_options(ssvg PRIVATE "/Zc:__cplusplus")
endif()

# Debug trace
target_compile_definitions(ssvg PRIVATE
    $<$<CONFIG:Debug>:SSVG_CONFIG_DEBUG=1>
)

target_link_libraries(ssvg
    PRIVATE
    stdutils
)

set_property(TARGET ssvg PROPERTY FOLDER "third_parties")
