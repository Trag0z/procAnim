#pragma once
#include "../pch.h"
#include "../Types.h"

struct Mesh {
    GLuint vao;
    GLuint numIndices;

    Mesh() : vao(0), numIndices(0) {}
    Mesh(std::vector<Vertex> vertices, std::vector<uint> indices);
    Mesh(const char* file);
    static void init();
    static Mesh simple();

  protected:
    using init_data_t = std::pair<std::vector<Vertex>, std::vector<uint>>;
    Mesh(std::vector<Vertex> vertices, std::vector<uint> indices, GLenum usage);
    Mesh(init_data_t initData, GLenum usage);
    Mesh(const char* file, GLenum usage);
    init_data_t loadDataFromFile(const char* file);

  private:
    static struct {
        GLuint vao;
        GLuint numIndices;
    } simpleMesh;

    Mesh(GLuint vao, GLuint numIndeces) : vao(vao), numIndices(numIndeces) {}
};

struct MutableMesh : Mesh {
    std::vector<Vertex> vertices;

    MutableMesh() : Mesh(), vertices(0) {}
    MutableMesh(std::vector<Vertex> vertices, std::vector<uint> indices);
    MutableMesh(const char* file);

    void update() const;

  private:
    MutableMesh(init_data_t initData);
};