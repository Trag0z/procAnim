#pragma once
#include "pch.h"
#include "Types.h"
#include "Animator.h"

struct Bone {
    std::string name;
    const Bone* parent;
    glm::mat4 bind_pose_transform;
    glm::mat4 inverse_bind_pose_transform;
    // Position and length in local space
    glm::vec4 tail;
    float length;
    // Radians around z-Axis
    float rotation = 0.0f;

    static constexpr size_t INDEX_NOT_FOUND = UINT_MAX;
    glm::mat4 get_transform() const;
};

struct RiggedMesh {
    std::vector<Vertex> vertices;

    VertexArrayData<ShaderVertex> vao;
    std::vector<ShaderVertex> shader_vertices;

    VertexArrayData<DebugShaderVertex> bones_vao;
    std::vector<DebugShaderVertex> bones_shader_vertices;

    std::vector<Bone> bones;

    std::vector<LimbAnimator> animators;

    Bone* find_bone(const char* name);
    void load_from_file(const char* file);
};
