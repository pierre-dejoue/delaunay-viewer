#pragma once

const char* vertex_shader_source = R"SRC(

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_color;
uniform mat4 mat_proj;
out vec3 color;
void main()
{
    gl_Position = mat_proj * vec4(v_pos, 1.0);
    color = v_color;
}

)SRC";

const char* fragment_shader_source = R"SRC(

in vec3 color;
layout (location = 0) out vec4 out_color;
void main()
{
    out_color = vec4(color, 1.0);
}

)SRC";
