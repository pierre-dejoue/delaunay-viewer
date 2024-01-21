if(TARGET Triangle)
    return()
endif()

message(STATUS "Third-party: Triangle")

#
# Shewchuk's Triangle library
#
#   https://cs.cmu.edu/~quake/triangle.html
#
include(FetchContent)
FetchContent_Declare(
    triangle
    GIT_REPOSITORY https://github.com/libigl/triangle.git
    GIT_TAG 6bbd92c7ddd6c803c403e005e1132eadb38fbe68
)
FetchContent_Populate(triangle)

add_library(Triangle STATIC ${triangle_SOURCE_DIR}/triangle.cpp)
target_include_directories(Triangle PUBLIC ${triangle_SOURCE_DIR}/)
target_compile_definitions(Triangle PRIVATE -DTRILIBRARY -DANSI_DECLARATORS)
if(WIN32)
  target_compile_definitions(Triangle PRIVATE -DNO_TIMER)
endif()
set_target_properties(Triangle PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_property(TARGET Triangle PROPERTY FOLDER "third_parties")
