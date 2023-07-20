#
# Delaunay libraries
#

# Dependencies
if(DELAUNAY_VIEWER_BUILD_POLY2TRI)
    include(poly2tri)
endif()
if(DELAUNAY_VIEWER_BUILD_CDT)
    include(cdt)
endif()

set(LIB_SOURCES
    src/dt_impl.cpp
    src/dt_interface.cpp
)

if(DELAUNAY_VIEWER_BUILD_POLY2TRI)
    list(APPEND LIB_SOURCES src/impl_poly2tri.cpp)
endif()
if(DELAUNAY_VIEWER_BUILD_CDT)
    list(APPEND LIB_SOURCES src/impl_cdt.cpp)
endif()

file(GLOB LIB_HEADERS include/dt/*.h src/*.h)

add_library(dt STATIC ${LIB_SOURCES} ${LIB_HEADERS})

target_include_directories(dt
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (DELAUNAY_VIEWER_BUILD_POLY2TRI)
    target_compile_definitions(dt
        PRIVATE
        BUILD_POLY2TRI
    )
endif()
if (DELAUNAY_VIEWER_BUILD_CDT)
    target_compile_definitions(dt
        PRIVATE
        BUILD_CDT
    )
endif()


target_link_libraries(dt
    PRIVATE
    shapes
    stdutils
    poly2tri
)

set_property(TARGET dt PROPERTY FOLDER "libraries")
