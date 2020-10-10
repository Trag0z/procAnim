#version 330 core
layout(location=0)in vec2 pos;
layout(location=1)in vec2 in_uv_coord;
layout(location=2)in uint bone_indices[2];
layout(location=3)in float bone_weights[2];

uniform mat3 camera;
uniform mat3 model;
uniform mat3 bone_transforms[15];

out vec2 uv_coord;

void main()
{
    mat3 bone=bone_transforms[bone_indices[0]]*bone_weights[0]+bone_transforms[bone_indices[1]]*bone_weights[1];
    vec3 local_pos=bone*vec3(pos,1.f);
    
    vec3 sim_pos=camera*model*local_pos;
    
    gl_Position=vec4(sim_pos.x,sim_pos.y,0.f,1.f);
    
    uv_coord=in_uv_coord;
}