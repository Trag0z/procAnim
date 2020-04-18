#version 330 core
layout (location = 0) in vec4 pos; 

uniform vec4 color;
out vec4 aColor;

void main()
{
    aColor = color;
    gl_Position = pos;
}