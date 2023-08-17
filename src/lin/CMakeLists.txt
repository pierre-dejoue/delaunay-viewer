#
# lin: Basic & generic linear algebra
#

include(compiler_options)

set(LIB_HEADERS
    include/lin/mat.h
    include/lin/vect.h
)

add_library(lin INTERFACE ${LIB_HEADERS})

target_include_directories(lin
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lin
    INTERFACE
    stdutils
)

set_property(TARGET lin PROPERTY FOLDER "libraries")
