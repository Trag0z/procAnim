#pragma once
#include "pch.h"
#include "Mesh.h"

using namespace MeshDetail;

void Mesh::init() {
    Mesh m = Mesh({{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
                   {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}},
                   {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
                   {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}}},
                  {0, 1, 2, 0, 3, 1});
    simpleMesh.vao = m.vao;
    simpleMesh.numIndices = m.numIndices;
}

Mesh::Mesh(std::vector<BasicVertex> vertices, std::vector<uint> indices) {
    numIndices = static_cast<GLuint>(indices.size());
    GLuint vbo, ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BasicVertex) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BasicVertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(BasicVertex),
        reinterpret_cast<void*>(offsetof(BasicVertex, uvCoord)));
    glEnableVertexAttribArray(1);

    // Reset vertex array binding for error safety
    glBindVertexArray(0);
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

    std::vector<BasicVertex> vertices;
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

RiggedMesh::RiggedMesh(const char* file) {
    // Load data from file
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        file, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);

    if (!scene) {
        printf("[ASSIMP] Error loading asset from %s: %s\n", file,
               importer.GetErrorString());
        SDL_assert(false);
    }

    auto convertMatrix = [](glm::mat4& out, aiMatrix4x4& in) {
        memcpy(&out, &in, sizeof(float) * 4 * 4);
        out = glm::transpose(out);
    };

    convertMatrix(globalInverseTransform,
                  scene->mRootNode->mTransformation.Inverse());

    SDL_assert(scene->HasMeshes());

    aiMesh& meshData = *scene->mMeshes[0];

#ifndef SHADER_DEBUG
    std::vector<Vertex> vertices;
#endif
    vertices.reserve(meshData.mNumVertices);
    aiVector3D* vertData = meshData.mVertices;
    for (size_t i = 0; i < meshData.mNumVertices; ++i) {
        vertices.push_back(
            {{vertData[i].x, vertData[i].y, 0.0f},
             {0.0f, 0.0f},
             {10, 10},       // So they reference the unity matrix by default
             {1.0f, 0.0f}}); // Or 0.5f, 0.5f?
    }

    debugVertices = new DebugVertex[vertices.size()];

    std::vector<uint> indices;
    indices.reserve(static_cast<size_t>(meshData.mNumFaces) * 3);
    for (size_t i = 0; i < meshData.mNumFaces; ++i) {
        aiFace& face = meshData.mFaces[i];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }
    numIndices = static_cast<GLuint>(indices.size());

    // Parse bones and sort weights/indices into vertex data
    const uint availableBones = maxBones - 1;
    boneNames[availableBones] = "unity";
    boneOffsets[availableBones] = glm::mat4(1.0f);
    SDL_assert(static_cast<size_t>(meshData.mNumBones) <= availableBones);

    struct WeightData {
        uint vertIndex, boneIndex;
        float weight;
    };
    std::vector<WeightData> weightData;

    for (uint i = 0; i < availableBones; ++i) {
        if (i >= meshData.mNumBones) {
            boneNames[i] = "null";
            boneOffsets[i] = glm::mat4(1.0f);
            continue;
        }
        aiBone& b = *meshData.mBones[i];

        boneNames[i] = b.mName.C_Str();
        convertMatrix(boneOffsets[i], b.mOffsetMatrix);

        for (uint j = 0; j < b.mNumWeights; ++j) {
            weightData.push_back(
                {b.mWeights[j].mVertexId, i, b.mWeights[j].mWeight});
        }
    }

    size_t* vertexBoneCounts = new size_t[vertices.size()];
    memset(vertexBoneCounts, 0, vertices.size() * sizeof(size_t));
    for (auto w : weightData) {
        size_t boneCount = vertexBoneCounts[w.vertIndex];
        if (boneCount == 2)
            continue;
        vertices[w.vertIndex].boneIndex[boneCount] = w.boneIndex;
        vertices[w.vertIndex].boneWeight[boneCount] = w.weight;
        ++boneCount;
    }
    delete[] vertexBoneCounts;

    // Upload data to GPU
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

#ifndef SHADER_DEBUG
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uvCoord)));
    glEnableVertexAttribArray(1);
    // boneIndex attribute
    glVertexAttribIPointer(
        2, 2, GL_UNSIGNED_INT, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, boneIndex)));
    glEnableVertexAttribArray(2);
    // boneWeight attribute
    glVertexAttribPointer(
        3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, boneWeight)));
    glEnableVertexAttribArray(3);
#else
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex) * vertices.size(), NULL,
                 GL_DYNAMIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex),
        reinterpret_cast<void*>(offsetof(DebugVertex, uvCoord)));
    glEnableVertexAttribArray(1);
#endif

    // Reset vertex array binding for error safety
    glBindVertexArray(0);
}
