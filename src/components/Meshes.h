#pragma once
#include "../pch.h"
#include "../Types.h"

struct Bone {
    struct VertexWeight {
        uint vertexId;
        float weight;
    };

    std::string name;
    glm::mat4 offsetMatrix;
    std::vector<VertexWeight> weights;

    Bone(){};
    Bone(aiBone& b);
};

struct Mesh {
    GLuint vao;
    GLuint numIndices;

    Mesh() : vao(0), numIndices(0) {}
    Mesh(const char* file);
    static void init();
    static Mesh simple();

  protected:
    Mesh(std::vector<Vertex> vertices, std::vector<uint> indices);
    void createVao(const std::vector<Vertex>& vertices,
                   const std::vector<uint>& indices, GLenum usage);

  private:
    static struct {
        GLuint vao;
        GLuint numIndices;
    } simpleMesh;

    Mesh(GLuint vao, GLuint numIndeces) : vao(vao), numIndices(numIndeces) {}
};

struct MutableMesh : Mesh {
    std::vector<Vertex> vertices;
    std::vector<Bone> bones;

    MutableMesh() : Mesh(), vertices(0), bones(0) {}
    MutableMesh(const char* file);

    void update() const;
};