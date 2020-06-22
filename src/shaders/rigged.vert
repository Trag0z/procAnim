#version 330 core
layout (location = 0) in vec4 pos; 
layout (location = 1) in vec2 in_uv_coord;

uniform mat4 projection;
uniform mat4 model;

out vec2 uv_coord;

void main()
{
    gl_Position = projection * model * pos;
    uv_coord = in_uv_coord;
}