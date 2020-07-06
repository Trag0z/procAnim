#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 in_uv_coord;

uniform mat3 camera;
uniform mat3 model;

out vec2 uv_coord; 

void main()
{
    vec3 sim_pos = camera * model * vec3(pos, 1.0f);
    gl_Position = vec4(sim_pos.x, sim_pos.y, 0.0f, 1.0f);
    uv_coord = in_uv_coord;
}