#pragma once
#include "../pch.h"
#include "../Types.h"

namespace MeshDetail {

struct BasicVertex {
    glm::vec3 position;
    glm::vec2 uvCoord;
};

struct Vertex {
    glm::vec3 position;
    glm::vec2 uvCoord;
    float boneIndex[2];
    float boneWeight[2];
};

} // namespace MeshDetail

using namespace MeshDetail;

struct Mesh {
    GLuint vao;
    GLuint numIndices;

    Mesh() : vao(0), numIndices(0) {}
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

    Mesh(GLuint vao, GLuint numIndeces) : vao(vao), numIndices(numIndeces) {}
};

struct RiggedMesh {
    GLuint vao;
    GLuint numIndices;

    glm::mat4 globalInverseTransform;

    static const uint maxBones = 11;
    std::string boneNames[maxBones];
    glm::mat4 boneOffsets[maxBones];

    RiggedMesh() : vao(0), numIndices(0) {}
    RiggedMesh(const char* file);
};
