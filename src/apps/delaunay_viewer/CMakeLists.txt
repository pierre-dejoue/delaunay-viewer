#
# Build the graphical application
#

# Dependencies
include(argagg)

configure_file(src/project_defines.h.in project_defines.h @ONLY)

set(GUI_SOURCES
    src/about_window.cpp
    src/drawing_settings.cpp
    src/gui_style.cpp
    src/main.cpp
    src/project.cpp
    src/renderer.cpp
    src/renderer_helpers.cpp
    src/settings.cpp
    src/settings_window.cpp
    src/shape_control_window.cpp
    src/viewport_window.cpp
)

file(GLOB GUI_HEADERS src/*.h)

add_executable(delaunay_viewer ${GUI_SOURCES} ${GUI_HEADERS})

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
    gui_base
    imgui
    argagg-lib
    dt
    shapes
    stdutils
    lin
    svg
)

install(TARGETS delaunay_viewer)

add_custom_target(delaunay_viewer_version
    $<TARGET_FILE:delaunay_viewer> --version
)
