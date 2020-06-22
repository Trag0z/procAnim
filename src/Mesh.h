#pragma once
#include "pch.h"
#include "Types.h"
#include "Animator.h"

struct Bone {
    std::string name;
    const Bone* parent;
    glm::mat4 bind_pose_transform;
    glm::mat4 inverse_bind_pose_transform;
    // Position and length in bone space
    glm::vec4 tail;
    float length;
    // Radians around z-Axis
    float rotation = 0.0f;

    glm::mat4 get_transform() const;
};

struct RiggedMesh {
    std::vector<RiggedVertex> vertices;

    VertexArray<RiggedShader::Vertex> vao;
    std::vector<RiggedShader::Vertex> shader_vertices;

    VertexArray<DebugShader::Vertex> bones_vao;
    std::vector<DebugShader::Vertex> bones_shader_vertices;

    std::vector<Bone> bones;

    Bone* find_bone(const char* name);
    void load_from_file(const char* file);
};
