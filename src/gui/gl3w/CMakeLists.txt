set(GL3W_SRC
    src/gl3w.c
)

set(GL3W_HDR
    include/GL/gl3w.h
    include/GL/glcorearb.h
    include/KHR/khrplatform.h
)

add_library(gl3w STATIC ${GL3W_SRC} ${GL3W_HDR})

target_include_directories(gl3w
    PUBLIC
    include
)

set_property(TARGET gl3w PROPERTY FOLDER "third_parties")
