#
# graphs: Graph data structures and algorihtms
#

include(compiler_options)

set(LIB_HEADERS
    include/graphs/graph.h
    include/graphs/graph_algos.h
)

add_library(graphs INTERFACE ${LIB_HEADERS})

target_include_directories(graphs
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(graphs
    INTERFACE
    stdutils
)

set_property(TARGET graphs PROPERTY FOLDER "libraries")
