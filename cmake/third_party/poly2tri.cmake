if(TARGET poly2tri)
    return()
endif()

message(STATUS "Third-party: poly2tri")

include(FetchContent)
FetchContent_Declare(
    poly2tri
#   GIT_REPOSITORY https://github.com/jhasse/poly2tri.git
#   GIT_TAG 81612cb108b54c14c695808f494f432990b279fd
    GIT_REPOSITORY https://github.com/pierre-dejoue/poly2tri.git
    GIT_TAG 145ac887356adfcf6a52c227f9d907cf6e77ec0e
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(poly2tri)

set_property(TARGET poly2tri PROPERTY FOLDER "third_parties")
