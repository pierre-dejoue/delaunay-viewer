#
# Unit tests
#
include(catch2)

set(UTESTS_SOURCES
    src/test_bounding_box.cpp
    src/test_sampling.cpp
    src/test_shapes.cpp
    src/test_vect.cpp
)

add_executable(utests_shapes ${UTESTS_SOURCES})

target_link_libraries(utests_shapes
    PRIVATE
    Catch2::Catch2WithMain
    shapes
)

set_property(TARGET utests_shapes PROPERTY FOLDER "tests")

add_custom_target(run_utests_shapes
    $<TARGET_FILE:utests_shapes> --skip-benchmarks
    COMMENT "Run shapes library UTests:"
)
