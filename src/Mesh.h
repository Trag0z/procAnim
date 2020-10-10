#pragma once
#include "pch.h"
#include "Types.h"
#include "Shaders.h"

struct Bone {
    std::string name;
    const Bone* parent;
    glm::mat3 bind_pose_transform;
    glm::mat3 inverse_bind_pose_transform;
    // Position and length in bone space
    glm::vec2 tail;
    float length;
    // Radians around z-Axis
    float rotation = 0.0f;

    glm::mat3 get_transform() const;
    glm::vec2 head() const;
};

struct RiggedMesh {
    VertexArray<RiggedShader::Vertex> vao;

    VertexArray<DebugShader::Vertex> bones_vao;
    std::vector<DebugShader::Vertex> bones_shader_vertices;

    std::vector<Bone> bones;

    Bone* find_bone(const char* name);
    void load_from_file(const char* file);
};
