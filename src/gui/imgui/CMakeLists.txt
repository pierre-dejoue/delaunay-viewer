# Dependencies
include(glfw)
include(imgui)

set(IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    src/imgui_helpers.cpp
    src/key_shortcut.cpp
)

if(DELAUNAY_VIEWER_IMGUI_DEMO)
    list(APPEND IMGUI_SRC ${imgui_SOURCE_DIR}/imgui_demo.cpp)
endif()

set(IMGUI_HDR
    include/config/imgui_config.h
    include/imgui/imgui.h
    include/imgui/key_shortcut.h
    ${imgui_SOURCE_DIR}/imconfig.h
    ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/imgui_internal.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h

)

add_library(imgui ${IMGUI_SRC} ${IMGUI_HDR})

target_include_directories(imgui
    PUBLIC
    include/
    include/config/
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    src/
)

target_compile_definitions(imgui
    PUBLIC
    IMGUI_USER_CONFIG="imgui_config.h"
)

target_link_libraries(imgui
    PUBLIC
    gui_base
    stdutils
    PRIVATE
    glfw
)

set_property(TARGET imgui PROPERTY FOLDER "third_parties")
