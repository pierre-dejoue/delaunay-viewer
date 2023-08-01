#
# Unit tests
#
include(catch2)

set(UTESTS_SOURCES
    src/test_span.cpp
)

add_executable(utests_stdutils ${UTESTS_SOURCES})

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
