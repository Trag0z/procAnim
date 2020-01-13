#version 330 core
layout (location = 0) in vec3 pos; 
layout (location = 1) in vec2 inUVCoord;
layout (location = 2) in vec2 boneIndex;
layout (location = 3) in vec2 boneWeight;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 bones[11];

out vec2 uvCoord;

void main()
{
    uvCoord = inUVCoord;
    // vec4 newVertex;
    int index;

    index = int(boneIndex.x);
    mat4 boneTransform = bones[index] * boneWeight.x;
    index = int(boneIndex.y);
    boneTransform += bones[index] * boneWeight.y;

    gl_Position = projection * model * (boneTransform * vec4(pos.xyz, 1.0f)); // just to be safe, since newVertex.w might not be 1.0f
}