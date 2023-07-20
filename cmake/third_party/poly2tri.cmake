if(TARGET poly2tri)
    return()
endif()

message(STATUS "Third-party: poly2tri")

include(FetchContent)
FetchContent_Declare(
    poly2tri
    GIT_REPOSITORY https://github.com/jhasse/poly2tri.git
    GIT_TAG 81612cb108b54c14c695808f494f432990b279fd
)
FetchContent_MakeAvailable(poly2tri)

set_property(TARGET poly2tri PROPERTY FOLDER "third_parties")

