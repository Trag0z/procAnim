#version 330 core
layout(location=0)in vec2 pos;

uniform mat3 camera;
uniform mat3 model;
uniform mat3 bone_transforms[15];

void main()
{
    mat3 bone=bone_transforms[gl_VertexID/2];
    vec3 local_pos=bone*vec3(pos,1.f);
    
    vec3 sim_pos=camera*model*local_pos;
    
    gl_Position=vec4(sim_pos.x,sim_pos.y,0.f,1.f);
}