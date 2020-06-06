#version 330 core
layout (location = 0) in vec4 pos; 
layout (location = 1) in vec2 inUVCoord;

uniform mat4 projection;
out vec2 uvCoord;

void main()
{
    gl_Position = projection * pos;
    uvCoord = inUVCoord;
}