if(TARGET poly2tri)
    return()
endif()

message(STATUS "Third-party: poly2tri")

include(FetchContent)
FetchContent_Declare(
    poly2tri
#    GIT_REPOSITORY https://github.com/jhasse/poly2tri.git
#    GIT_TAG 81612cb108b54c14c695808f494f432990b279fd
    GIT_REPOSITORY https://github.com/pierre-dejoue/poly2tri.git
    GIT_TAG 5d3795006bd5b79ee8da9c79eba743191c8f9bfe
)
FetchContent_Populate(poly2tri)

# When specifying an out-of-tree source a binary directory must be explicitly specified (2nd argument.)
add_subdirectory(${poly2tri_SOURCE_DIR} poly2tri EXCLUDE_FROM_ALL)

set_property(TARGET poly2tri PROPERTY FOLDER "third_parties")

