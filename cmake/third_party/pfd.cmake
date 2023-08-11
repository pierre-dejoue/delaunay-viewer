if(TARGET pfd)
    return()
endif()

message(STATUS "Third-party: pfd")

include(FetchContent)
FetchContent_Declare(
    pfd
    GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git
    GIT_TAG 7f852d88a480020d7f91957cbcefe514fc95000c
)
FetchContent_MakeAvailable(pfd)

# To make it visible in the IDE
add_custom_target(pfd SOURCES
    ${pfd_SOURCE_DIR}/portable-file-dialogs.h
)

set_property(TARGET pfd PROPERTY FOLDER "third_parties")
