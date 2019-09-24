#version 330 core
layout (location = 0) in vec3 pos; 
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUVCoord;

out vec3 color;
out vec2 uvCoord;

void main()
{
    gl_Position = vec4(pos, 1.0);
    color = inColor;
    uvCoord = inUVCoord;
}