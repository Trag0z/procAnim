#pragma once
#include "pch.h"
#include "Types.h"
#include "Animator.h"

struct Bone {
    std::string name;
    size_t parent;
    glm::mat4 bind_pose_transform;
    glm::mat4 inverse_bind_pose_transform;
    // Position in local space
    glm::vec4 tail;
    // Radians around z-Axis
    float rotation = 0.0f;
    float length;

    static constexpr size_t INDEX_NOT_FOUND = UINT_MAX;
    inline bool has_parent() const { return parent != INDEX_NOT_FOUND; }
};

struct RiggedMesh {
    std::vector<Vertex> vertices;

    VertexArrayData<ShaderVertex> vao;
    std::vector<ShaderVertex> shader_vertices;

    VertexArrayData<DebugShaderVertex> bones_vao;
    std::vector<DebugShaderVertex> bones_shader_vertices;

    std::vector<Bone> bones;

    std::vector<LimbAnimator> animators;

    size_t find_bone_index(const char* name) const;
    void load_from_file(const char* file);
};
