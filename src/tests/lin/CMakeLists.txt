#
# Unit tests for shapes and graphs
#
include(catch2)

set(UTESTS_SOURCES
    src/test_mat.cpp
)

add_executable(utests_linear_algebra ${UTESTS_SOURCES})

set_target_warnings(utests_linear_algebra ON)

target_link_libraries(utests_linear_algebra
    PRIVATE
    Catch2::Catch2WithMain
    lin
)

set_property(TARGET utests_linear_algebra PROPERTY FOLDER "tests")

add_custom_target(run_utests_linear_algebra
    $<TARGET_FILE:utests_linear_algebra> --skip-benchmarks
    COMMENT "Run linear algebra (lin) library UTests:"
)
