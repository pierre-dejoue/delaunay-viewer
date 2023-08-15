if(TARGET cdt)
    return()
endif()

message(STATUS "Third-party: cdt")

include(FetchContent)
FetchContent_Declare(
    cdt
    GIT_REPOSITORY https://github.com/artem-ogre/CDT.git
    GIT_TAG 1.3.0
)
FetchContent_Populate(cdt)

# When specifying an out-of-tree source a binary directory must be explicitly specified (2nd argument.)
add_subdirectory(${cdt_SOURCE_DIR}/CDT CDT EXCLUDE_FROM_ALL)    # The target is called CDT::CDT

set_property(TARGET CDT PROPERTY FOLDER "third_parties")

