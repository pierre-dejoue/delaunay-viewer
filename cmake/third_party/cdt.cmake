if(TARGET cdt)
    return()
endif()

message(STATUS "Third-party: cdt")

include(FetchContent)
FetchContent_Declare(
    cdt
    GIT_REPOSITORY https://github.com/artem-ogre/CDT.git
    GIT_TAG 1.3.0
    SOURCE_SUBDIR CDT
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(cdt)     # The target CDT is aliased with CDT::CDT

set_property(TARGET CDT PROPERTY FOLDER "third_parties")
