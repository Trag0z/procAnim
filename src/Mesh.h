#pragma once
#include "pch.h"
#include "Types.h"
#include "Shaders.h"

struct Bone {
    std::string name;
    const Bone* parent;

    glm::mat3 bind_pose_transform; // Transforms from bone space to mesh space
    glm::mat3
        inverse_bind_pose_transform; // Transforms from mesh space to bone space

    // Position of the tail in the bones local space
    glm::vec2 tail;
    float length;

    // Radians around z-Axis
    float rotation = 0.0f;

    // The returned matrix applies the transformation for this bone (and by
    // extension, all parents of the bone) to a vector in mesh space
    glm::mat3 transform() const;

    // Current position of the bone's head (affacted by parent bones) in mesh
    // space
    glm::vec2 head() const;
};

struct Mesh {
    VertexArray<RiggedShader::Vertex> vao;
    VertexArray<RiggedDebugShader::Vertex> bones_vao;

    std::vector<Bone> bones;

    Bone* find_bone(const char* name);
    void load_from_file(const char* file);
};
