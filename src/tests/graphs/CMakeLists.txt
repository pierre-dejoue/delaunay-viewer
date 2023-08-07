#
# Unit tests
#
include(catch2)

set(UTESTS_SOURCES
    src/test_graphs.cpp
)

add_executable(utests_graphs ${UTESTS_SOURCES})

set_target_warnings(utests_graphs ON)

target_link_libraries(utests_graphs
    PRIVATE
    Catch2::Catch2WithMain
    graphs
)

set_property(TARGET utests_graphs PROPERTY FOLDER "tests")

add_custom_target(run_utests_graphs
    $<TARGET_FILE:utests_graphs> --skip-benchmarks
    COMMENT "Run graphs library UTests:"
)
