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
    simpleMesh.numIndices = m.num_indices;
}

Mesh::Mesh(std::vector<BasicVertex> vertices, std::vector<uint> indices) {
    num_indices = static_cast<GLuint>(indices.size());
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
    // uv_coord attribute
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(BasicVertex),
        reinterpret_cast<void*>(offsetof(BasicVertex, uv_coord)));
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

RiggedMesh RiggedMesh::load_from_file(const char* file) {
    // Load data from file
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

    // Import vertex data
    RiggedMesh result;

    result.vertices.reserve(meshData.mNumVertices);
    result.shader_vertices.reserve(meshData.mNumVertices);

    aiVector3D* vertData = meshData.mVertices;
    for (size_t i = 0; i < meshData.mNumVertices; ++i) {
        result.vertices.push_back({{vertData[i].x, vertData[i].y, 0.0f},
                                   {0.0f, 0.0f},
                                   {0, 0},
                                   {0.0f, 0.0f}}); // Or 0.5f, 0.5f?
        result.shader_vertices.push_back({});
    }

    std::vector<uint> indices;
    indices.reserve(static_cast<size_t>(meshData.mNumFaces) * 3);
    for (size_t i = 0; i < meshData.mNumFaces; ++i) {
        aiFace& face = meshData.mFaces[i];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }
    result.num_indices = static_cast<GLuint>(indices.size());

    // Parse bones and sort weights/indices into vertex data
    struct WeightData {
        uint vert_index, bone_index;
        float weight;
    };
    std::vector<WeightData> weight_data;

    // TODO: Maybe remove later
    auto convertMatrix = [](glm::mat4& out, aiMatrix4x4& in) {
        memcpy(&out, &in, sizeof(float) * 4 * 4);
        out = glm::transpose(out);
    };

    // Create bones and populate weight_data
    result.bones.reserve(meshData.mNumBones);
    for (uint i = 0; i < meshData.mNumBones; ++i) {
        RiggedMesh::Bone b;
        b.rotation = glm::mat4(1.0f);
        aiBone& ai_bone = *meshData.mBones[i];

        b.name = ai_bone.mName.C_Str();
        convertMatrix(b.inverse_transform, ai_bone.mOffsetMatrix);

        for (uint j = 0; j < ai_bone.mNumWeights; ++j) {
            weight_data.push_back({ai_bone.mWeights[j].mVertexId, i,
                                   ai_bone.mWeights[j].mWeight});
        }

        result.bones.push_back(b);
    }

    // Find bone partents
    aiNode* root = scene->mRootNode;

    for (auto& b : result.bones) {
        aiNode* node = root->FindNode(b.name.c_str());
        b.parent = result.find_bone_index(node->mParent->mName.C_Str());
    }

    // Assign bones and weights to vertices
    auto& vertices = result.vertices;
    size_t* vertex_bone_counts = new size_t[vertices.size()];
    memset(vertex_bone_counts, 0, vertices.size() * sizeof(size_t));

    for (auto w : weight_data) {
        size_t bone_count = vertex_bone_counts[w.vert_index];
        if (bone_count == MAX_BONES_PER_VERTEX)
            continue;

        vertices[w.vert_index].bone_index[bone_count] = w.bone_index;
        vertices[w.vert_index].bone_weight[bone_count] = w.weight;
        ++bone_count;
    }
    delete[] vertex_bone_counts;

    // Upload data to GPU
    glGenVertexArrays(1, &result.vao);
    glBindVertexArray(result.vao);

    // Create index buffer
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    // Create vertex buffer
    glGenBuffers(1, &result.vbo);

    glBindBuffer(GL_ARRAY_BUFFER, result.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ShaderVertex) * vertices.size(), NULL,
                 GL_DYNAMIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex),
        reinterpret_cast<void*>(offsetof(ShaderVertex, uv_coord)));
    glEnableVertexAttribArray(1);

#ifndef CPU_RENDERING
    // boneIndex attribute
    glVertexAttribIPointer(
        2, 2, GL_UNSIGNED_INT, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, bone_index)));
    glEnableVertexAttribArray(2);
    // boneWeight attribute
    glVertexAttribPointer(
        3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, bone_weight)));
    glEnableVertexAttribArray(3);
#endif

    // Reset vertex array binding for error safety
    glBindVertexArray(0);

    return result;
}

uint RiggedMesh::find_bone_index(const char* str) const {
    for (uint i = 0; i < bones.size(); ++i) {
        if (bones[i].name.compare(str) == 0) {
            return i;
        }
    }
    return UINT_MAX;
}