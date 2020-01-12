#pragma once
#include "pch.h"
#include "Meshes.h"

Bone::Bone(aiBone& b) : name(b.mName.C_Str()) {
    memcpy(&offsetMatrix, &b.mOffsetMatrix, sizeof(float) * 4 * 4);
    offsetMatrix = glm::transpose(offsetMatrix);
    for (size_t i = 0; i < b.mNumWeights; ++i) {
        weights.push_back({b.mWeights[i].mVertexId, b.mWeights[i].mWeight});
    }
}

void Mesh::init() {
    Mesh m = Mesh({{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
                   {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}},
                   {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
                   {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}}},
                  {0, 1, 2, 0, 3, 1});
    simpleMesh.vao = m.vao;
    simpleMesh.numIndices = m.numIndices;
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint> indices) {
    numIndices = static_cast<GLuint>(indices.size());
    createVao(vertices, indices, GL_STATIC_DRAW);
}

Mesh::Mesh(const char* file) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        file, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);

    if (!scene) {
        printf("[ASSIMP] Error loading asset from %s: %s\n", file,
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

    Mesh(vertices, indices);
}

Mesh Mesh::simple() { return {simpleMesh.vao, simpleMesh.numIndices}; }

void Mesh::createVao(const std::vector<Vertex>& vertices,
                     const std::vector<uint>& indices, GLuint usage) {
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

MutableMesh::MutableMesh(const char* file) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        file, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);

    if (!scene) {
        printf("[ASSIMP] Error loading asset from %s: %s\n", file,
               importer.GetErrorString());
        SDL_assert(false);
    }

    SDL_assert(scene->HasMeshes());

    aiMesh& meshData = *scene->mMeshes[0];

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

    bones.reserve(static_cast<size_t>(meshData.mNumBones));
    for (size_t i = 0; i < meshData.mNumBones; ++i) {
        bones.push_back(*meshData.mBones[i]);
    }

    numIndices = static_cast<GLuint>(indices.size());
    createVao(vertices, indices, GL_DYNAMIC_DRAW);
}

void MutableMesh::update() const {
    glBindVertexArray(vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
                 vertices.data(), GL_DYNAMIC_DRAW);
}
