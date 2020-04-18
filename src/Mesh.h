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
    GLuint vao, vbo;
    GLuint num_indices;

    GLuint bones_vao, bones_vbo;

    std::vector<Vertex> vertices;

    struct ShaderVertex {
        glm::vec4 pos;
        glm::vec2 uv_coord;
    };
    std::vector<ShaderVertex> shader_vertices;

    struct DebugShaderVertex {
        glm::vec4 pos;
    };
    std::vector<DebugShaderVertex> bones_shader_vertices;

    struct Bone {
        std::string name;
        size_t parent;
        glm::mat4 inverse_transform;
        glm::mat4 rotation;
        glm::vec3 head, tail;
        float length;

        static constexpr size_t INDEX_NOT_FOUND = UINT_MAX;
        inline bool has_parent() const { return parent != INDEX_NOT_FOUND; }
    };
    std::vector<Bone> bones;

    size_t find_bone_index(const char* name) const;
    static RiggedMesh load_from_file(const char* file);
};
