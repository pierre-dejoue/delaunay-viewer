#
# shapes: Basic & generic 2D/3D geometrical shapes
#

include(compiler_options)

set(LIB_SOURCES
    src/bounding_box.cpp
    src/io.cpp
    src/path.cpp
    src/point_cloud.cpp
    src/point.cpp
)

add_library(shapes STATIC ${LIB_SOURCES})

target_include_directories(shapes
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_warnings(shapes ON)

target_link_libraries(shapes
    PRIVATE
    stdutils
)

set_property(TARGET shapes PROPERTY FOLDER "libraries")
