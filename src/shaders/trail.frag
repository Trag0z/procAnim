#version 330 core
uniform vec4 old_color;
uniform vec4 recent_color;

in float age;

out vec4 frag_color;

void main()
{
    frag_color=age*old_color+(1.-age)*recent_color;
}