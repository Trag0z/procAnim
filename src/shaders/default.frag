#version 330 core
in vec2 uvCoord;

out vec4 fragColor;

uniform sampler2D texture1;

void main()
{    
    fragColor = texture(texture1, uvCoord);
}