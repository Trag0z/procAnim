#version 330 core
layout (location = 0) in vec3 pos; 
layout (location = 1) in vec2 inUVCoord;
layout (location = 2) in uvec2 boneIndex;
layout (location = 3) in vec2 boneWeight;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 bones[11];

out vec2 uvCoord;

void main()
{
    uvCoord = inUVCoord;

    mat4 boneTransform = bones[boneIndex.x] * boneWeight.x;
    boneTransform += bones[boneIndex.y] * boneWeight.y;

    gl_Position = projection * model * boneTransform * vec4(pos.xyz, 1.0f); 
}