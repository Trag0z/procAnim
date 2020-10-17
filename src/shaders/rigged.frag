#version 330 core
in vec2 uv_coord;

uniform sampler2D texture1;

out vec4 frag_color;

void main()
{
    frag_color=texture(texture1,uv_coord);
}