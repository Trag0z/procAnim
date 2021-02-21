#version 330 core
layout(location=0)in vec2 pos;
layout(location=1)in float in_age;

uniform mat3 camera;
uniform mat3 model;

out float age;

void main()
{
    vec3 sim_pos=camera*model*vec3(pos,1.f);
    gl_Position=vec4(sim_pos.x,sim_pos.y,0.f,1.f);
    
    age=in_age;
}