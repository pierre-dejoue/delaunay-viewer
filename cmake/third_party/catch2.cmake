if(TARGET Catch2::Catch2WithMain)
    return()
endif()

message(STATUS "Third-party: catch2")

Include(FetchContent)
FetchContent_Populate(
    catch2
    QUIET
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0
)

#
# Catch2 lib with main()
#
add_library(Catch2WithMain STATIC
    ${catch2_SOURCE_DIR}/extras/catch_amalgamated.cpp
    ${catch2_SOURCE_DIR}/extras/catch_amalgamated.hpp
)

target_include_directories(Catch2WithMain
    PUBLIC
    ${catch2_SOURCE_DIR}/extras
)

add_library(Catch2::Catch2WithMain ALIAS Catch2WithMain)

#
# Catch2 lib without main()
#
add_library(Catch2WithoutMain STATIC
    ${catch2_SOURCE_DIR}/extras/catch_amalgamated.cpp
    ${catch2_SOURCE_DIR}/extras/catch_amalgamated.hpp
)

target_compile_definitions(Catch2WithoutMain
    PRIVATE
    CATCH_AMALGAMATED_CUSTOM_MAIN
)

target_include_directories(Catch2WithoutMain
    PUBLIC
    ${catch2_SOURCE_DIR}/extras
)

add_library(Catch2::Catch2WithoutMain ALIAS Catch2WithoutMain)
