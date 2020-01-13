#version 330 core
layout (location = 0) in vec4 pos; 
layout (location = 1) in vec2 inUVCoord;

out vec2 uvCoord;

void main()
{
    gl_Position = pos;
    uvCoord = inUVCoord;
}