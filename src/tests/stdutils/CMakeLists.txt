#
# Unit tests
#
include(catch2)

set(UTESTS_SOURCES
    src/test_algorithm.cpp
    src/test_io.cpp
    src/test_locked_buffer.cpp
    src/test_memory.cpp
    src/test_platform.cpp
    src/test_range.cpp
    src/test_span.cpp
    src/test_stats.cpp
    src/test_string.cpp
    src/test_testing.cpp
    src/test_type_traits.cpp
)

add_executable(utests_stdutils ${UTESTS_SOURCES})

set_target_warnings(utests_stdutils ON)

target_link_libraries(utests_stdutils
    PRIVATE
    Catch2::Catch2WithMain
    stdutils
)

set_property(TARGET utests_stdutils PROPERTY FOLDER "tests")

add_custom_target(run_utests_stdutils
    $<TARGET_FILE:utests_stdutils> --skip-benchmarks
    COMMENT "Run stdutils library UTests:"
)
