#
# Build the graphical application
#

# Dependencies
include(pfd)
add_subdirectory(gl3w)
add_subdirectory(imgui)

configure_file(src/version.h.in version.h @ONLY)

set(GUI_SOURCES
    src/draw_shape.cpp
    src/main.cpp
    src/opengl_and_glfw.cpp
    src/settings.cpp
    src/settings_window.cpp
    src/shape_control_window.cpp
    src/shape_draw_window.cpp
    src/version.cpp
)

file(GLOB GUI_HEADERS src/*.h)

add_executable(delaunay_viewer ${GUI_SOURCES} ${GUI_HEADERS})

if(DELAUNAY_VIEWER_IMGUI_DEMO)
    target_compile_definitions(delaunay_viewer
        PRIVATE
        DELAUNAY_VIEWER_IMGUI_DEMO_FLAG
    )
endif()

if(APPLE)
    # OpenGL API is deprecated since macOS 10.14
    target_compile_definitions(delaunay_viewer
        PUBLIC
        GL_SILENCE_DEPRECATION
    )
endif()

target_include_directories(delaunay_viewer
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_warnings(delaunay_viewer ON)

target_link_libraries(delaunay_viewer
    PRIVATE
    dt
    gl3w
    imgui
    pfd
    stdutils
    svg
)

install(TARGETS delaunay_viewer)
