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
if(DELAUNAY_VIEWER_BUILD_TRIANGLE)
    include(triangle)
endif()

set(LIB_SOURCES
    src/dt_impl.cpp
    src/dt_interface.cpp
)

file(GLOB LIB_HEADERS include/dt/*.h src/*.h)

add_library(dt STATIC ${LIB_SOURCES} ${LIB_HEADERS})

target_include_directories(dt
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_warnings(dt ON)

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
if(DELAUNAY_VIEWER_BUILD_TRIANGLE)
    target_compile_definitions(dt
        PRIVATE
        BUILD_TRIANGLE
    )
endif()

set(THIRD_PARTY_LIBS)
if(DELAUNAY_VIEWER_BUILD_POLY2TRI)
    list(APPEND THIRD_PARTY_LIBS poly2tri)
endif()
if(DELAUNAY_VIEWER_BUILD_CDT)
    list(APPEND THIRD_PARTY_LIBS CDT::CDT)
endif()
if(DELAUNAY_VIEWER_BUILD_TRIANGLE)
    list(APPEND THIRD_PARTY_LIBS Triangle)
endif()

target_link_libraries(dt
    PRIVATE
    shapes
    stdutils
    ${THIRD_PARTY_LIBS}
)

set_property(TARGET dt PROPERTY FOLDER "libraries")
