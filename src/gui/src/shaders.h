#pragma once

const char* vertex_shader_source = R"SRC(

layout (location = 0) in vec3 v_pos;
uniform vec4 uni_color;
uniform mat4 mat_proj;
out vec4 color;
void main()
{
    gl_Position = mat_proj * vec4(v_pos, 1.0);
    color = uni_color;
}

)SRC";

const char* fragment_shader_source = R"SRC(

in vec4 color;
layout (location = 0) out vec4 out_color;
void main()
{
    out_color = color;
}

)SRC";
