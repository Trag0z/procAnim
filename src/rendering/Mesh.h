#pragma once
#include "../pch.h"
#include "../Types.h"
#include "Shaders.h"

struct Mesh;
struct RiggedMesh;

void load_character_model_from_file(const char* path, Mesh& body_mesh,
                                    RiggedMesh& rigged_mesh);

class Bone {
  public:
    // Radians around z-Axis
    float rotation = 0.0f;
    float length;

    const std::string& name() const noexcept;
    const Bone* parent() const noexcept;
    const glm::mat3& bind_pose_transform() const;
    const glm::mat3& inverse_bind_pose_transform() const;

    // The returned matrix applies the transformation for this bone (and by
    // extension, all parents of the bone) to a vector in mesh space.
    glm::mat3 transform() const;

    // Current position of the bone's head (affacted by parent bones) in mesh
    // space.
    glm::vec2 head() const;
    // The end of the bone that is affected by it's rotation.
    glm::vec2 tail() const;

    glm::vec2 tail_bind_pose() const noexcept;

  private:
    std::string name_;
    const Bone* parent_;

    glm::mat3 bind_pose_transform_; // Transforms from bone space to mesh space
    glm::mat3 inverse_bind_pose_transform_; // Transforms from mesh space to
                                            // bone space

    // Position of the tail in the bones local space
    glm::vec2 tail_;
    float original_length;

    friend void load_character_model_from_file(const char* path,
                                               Mesh& body_mesh,
                                               RiggedMesh& rigged_mesh);
};

struct Mesh {
    VertexArray<TexturedShader::Vertex> vao;
};

struct RiggedMesh {
    VertexArray<RiggedShader::Vertex> vao;
    VertexArray<BoneShader::Vertex> bones_vao;

    std::vector<Bone> bones;

    Bone* find_bone(const char* name);
};
