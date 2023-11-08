#
# Unit tests for shapes and graphs
#
include(catch2)

configure_file(src/trace.h.in trace.h @ONLY)

set(UTESTS_SOURCES
    src/test_bounding_box.cpp
    src/test_graphs.cpp
    src/test_sampling.cpp
    src/test_shapes.cpp
    src/test_union_find.cpp
    src/test_vect.cpp
    src/trace.cpp
)

add_executable(utests_shapes ${UTESTS_SOURCES})

set_target_warnings(utests_shapes ON)

target_include_directories(utests_shapes
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(utests_shapes
    PRIVATE
    Catch2::Catch2WithMain
    graphs
    shapes
)

set_property(TARGET utests_shapes PROPERTY FOLDER "tests")

add_custom_target(run_utests_shapes
    $<TARGET_FILE:utests_shapes> --skip-benchmarks
    COMMENT "Run shapes library UTests:"
)
