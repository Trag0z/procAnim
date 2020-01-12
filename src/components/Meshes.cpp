#pragma once
#include "pch.h"
#include "Meshes.h"

void Mesh::init() {
    Mesh m = Mesh({{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
                   {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}},
                   {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
                   {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}}},
                  {0, 1, 2, 0, 3, 1});
    simpleMesh.vao = m.vao;
    simpleMesh.numIndices = m.numIndices;
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint> indices)
    : Mesh(vertices, indices, GL_STATIC_DRAW) {}

Mesh::Mesh(const char* file) : Mesh(file, GL_STATIC_DRAW) {}

Mesh::Mesh(const char* file, GLenum usage)
    : Mesh(loadDataFromFile(file), usage) {}

Mesh::Mesh(init_data_t initData, GLenum usage)
    : Mesh(initData.first, initData.second, usage) {}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint> indices,
           GLuint usage)
    : numIndices(static_cast<GLuint>(indices.size())) {
    GLuint vbo, ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
                 vertices.data(), usage);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uvCoord)));
    glEnableVertexAttribArray(1);

    // Reset vertex array binding for error safety
    glBindVertexArray(0);
}

Mesh Mesh::simple() { return {simpleMesh.vao, simpleMesh.numIndices}; }

MutableMesh::MutableMesh(std::vector<Vertex> vertices,
                         std::vector<uint> indices)
    : Mesh(vertices, indices, GL_DYNAMIC_DRAW), vertices(std::move(vertices)) {}

MutableMesh::MutableMesh(const char* file)
    : MutableMesh(loadDataFromFile(file)) {}

MutableMesh::MutableMesh(init_data_t initData)
    : Mesh(initData, GL_DYNAMIC_DRAW), vertices(initData.first) {}

void MutableMesh::update() const {
    glBindVertexArray(vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
                 vertices.data(), GL_DYNAMIC_DRAW);
}

Mesh::init_data_t Mesh::loadDataFromFile(const char* file) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        file, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);

    if (!scene) {
        printf("[ASSIMP] Error loading asset from %s: %s", file,
               importer.GetErrorString());
        SDL_assert(false);
    }

    SDL_assert(scene->HasMeshes());

    aiMesh& meshData = *scene->mMeshes[0];

    std::vector<Vertex> vertices;
    vertices.reserve(meshData.mNumVertices);
    aiVector3D* vertData = meshData.mVertices;
    for (size_t i = 0; i < meshData.mNumVertices; ++i) {
        vertices.push_back(
            {{vertData[i].x, vertData[i].y, 0.0f}, {0.0f, 0.0f}});
    }

    std::vector<uint> indices;
    indices.reserve(static_cast<size_t>(meshData.mNumFaces) * 3);
    for (size_t i = 0; i < meshData.mNumFaces; ++i) {
        aiFace& face = meshData.mFaces[i];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    return {vertices, indices};
}