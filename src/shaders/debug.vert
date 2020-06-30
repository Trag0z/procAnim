#version 330 core
layout (location = 0) in vec2 pos; 

uniform mat3 camera;
uniform mat3 model;

void main()
{
    vec3 sim_pos = camera * model * vec3(pos, 1.0f);
    gl_Position = vec4(sim_pos.x, sim_pos.y, 0.0f, sim_pos.z);
}