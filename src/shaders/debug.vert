#version 330 core
layout (location = 0) in vec4 pos; 

uniform mat4 projection;
uniform mat4 model;
uniform vec4 color;
out vec4 aColor;

void main()
{
    gl_Position = projection * model * pos;
    aColor = color;
}