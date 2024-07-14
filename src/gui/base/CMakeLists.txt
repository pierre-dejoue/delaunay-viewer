#
# Base for graphical applications
#

set(GUI_BASE_SOURCES
    src/opengl_and_glfw.cpp
)

file(GLOB GUI_BASE_HEADERS include/base/*.h)

add_library(gui_base ${GUI_BASE_SOURCES} ${GUI_BASE_HEADERS})

if(APPLE)
    # OpenGL API is deprecated since macOS 10.14
    target_compile_definitions(gui_base
        PUBLIC
        GL_SILENCE_DEPRECATION
    )
endif()

target_include_directories(gui_base
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_warnings(gui_base ON)

target_link_libraries(gui_base
    PUBLIC
    gl3w
    glfw
    stdutils
    lin
    shapes
)
