#pragma once
#include "pch.h"
#include "Types.h"

namespace MeshDetail {

constexpr size_t MAX_BONES_PER_VERTEX = 2;

struct BasicVertex {
    glm::vec3 position;
    glm::vec2 uv_coord;
};

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv_coord;
    GLuint bone_index[MAX_BONES_PER_VERTEX];
    float bone_weight[MAX_BONES_PER_VERTEX];
};

} // namespace MeshDetail

using namespace MeshDetail;

struct Mesh {
    GLuint vao;
    GLuint num_indices;

    Mesh() : vao(0), num_indices(0) {}
    Mesh(const char* file);
    static void init();
    static Mesh simple();

  protected:
    Mesh(std::vector<BasicVertex> vertices, std::vector<uint> indices);

  private:
    static struct {
        GLuint vao;
        GLuint numIndices;
    } simpleMesh;

    Mesh(GLuint vao, GLuint numIndeces) : vao(vao), num_indices(numIndeces) {}
};

struct RiggedMesh {
    GLuint vao;
    GLuint vbo;
    GLuint num_indices;

    std::vector<Vertex> vertices;

    struct ShaderVertex {
        glm::vec4 pos;
        glm::vec2 uv_coord;
    };
    std::vector<ShaderVertex> shader_vertices;

    struct Bone {
        std::string name;
        uint parent;
        glm::mat4 inverse_transform;
        glm::mat4 rotation;
    };
    std::vector<Bone> bones;

    uint find_bone_index(const char* name) const;
    static RiggedMesh load_from_file(const char* file);
};
