#
# Abstract GUI
#
#   Concepts that belong to the GUI interface but do not depend on any graphical library
#

set(GUI_ABSTRACT_HEADERS
    include/gui/abstract/canvas.h
    include/gui/abstract/color_data.h
    include/gui/abstract/window_layout.h
)

add_library(gui_abstract INTERFACE ${GUI_ABSTRACT_HEADERS})

target_include_directories(gui_abstract
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gui_abstract
    INTERFACE
    shapes
    stdutils
)
