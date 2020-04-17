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

    aiMesh& mesh_data = *scene->mMeshes[0];

    std::vector<BasicVertex> vertices;
    vertices.reserve(mesh_data.mNumVertices);
    aiVector3D* vertData = mesh_data.mVertices;
    for (size_t i = 0; i < mesh_data.mNumVertices; ++i) {
        vertices.push_back(
            {{vertData[i].x, vertData[i].y, 0.0f}, {0.0f, 0.0f}});
    }

    std::vector<uint> indices;
    indices.reserve(static_cast<size_t>(mesh_data.mNumFaces) * 3);
    for (size_t i = 0; i < mesh_data.mNumFaces; ++i) {
        aiFace& face = mesh_data.mFaces[i];
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

    aiMesh& mesh_data = *scene->mMeshes[0];

    // Import vertex data
    RiggedMesh result;

    result.vertices.reserve(mesh_data.mNumVertices);
    result.shader_vertices.reserve(mesh_data.mNumVertices);

    aiVector3D* vertData = mesh_data.mVertices;
    aiVector3D* uvCoords = mesh_data.mTextureCoords[0];
    SDL_assert(uvCoords);

    for (size_t i = 0; i < mesh_data.mNumVertices; ++i) {
        result.vertices.push_back({{vertData[i].x, vertData[i].y, 0.0f},
                                   {uvCoords[i].x, 1.0f - uvCoords[i].y},
                                   {0, 0},
                                   {0.0f, 0.0f}}); // Or 0.5f, 0.5f?
        result.shader_vertices.push_back({});
    }

    std::vector<uint> indices;
    indices.reserve(static_cast<size_t>(mesh_data.mNumFaces) * 3);
    for (size_t i = 0; i < mesh_data.mNumFaces; ++i) {
        aiFace& face = mesh_data.mFaces[i];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }
    result.num_indices = static_cast<GLuint>(indices.size());

    // Parse bones and sort weights/indices into vertex data
    struct WeightData {
        size_t vert_index, bone_index;
        float weight;
    };
    std::vector<WeightData> weight_data;

    // TODO: Maybe remove later
    auto convert_to_column_major = [](glm::mat4& out, aiMatrix4x4& in) {
        memcpy(&out, &in, sizeof(float) * 4 * 4);
        out = glm::transpose(out);
    };

    // Create bones and populate weight_data
    result.bones.reserve(mesh_data.mNumBones);
    for (uint i = 0; i < mesh_data.mNumBones; ++i) {
        RiggedMesh::Bone b;
        b.rotation = glm::mat4(1.0f);

        aiBone& ai_bone = *mesh_data.mBones[i];
        b.name = ai_bone.mName.C_Str();
        convert_to_column_major(b.inverse_transform, ai_bone.mOffsetMatrix);
        b.length = 0.0f;

        for (uint j = 0; j < ai_bone.mNumWeights; ++j) {
            weight_data.push_back({ai_bone.mWeights[j].mVertexId, i,
                                   ai_bone.mWeights[j].mWeight});
        }

        result.bones.push_back(b);
    }

    // Find bone parents
    aiNode* root = scene->mRootNode;

    for (auto& b : result.bones) {
        aiNode* node = root->FindNode(b.name.c_str());
        b.parent = result.find_bone_index(node->mParent->mName.C_Str());

        if (node->mNumChildren > 0) {
            auto& child_transform = node->mChildren[0]->mTransformation;
            b.length = glm::length(glm::vec3(
                child_transform.a4, child_transform.b4, child_transform.c4));
        }
    }

    // Sort array so parents are always before children
    // NOTE: Does not take into account multiple children on the same bone,
    // wrong if there are longer chains of inheritence
    // TODO: clean this up, probably change the whole function
    bool sorted = false;
    while (!sorted) {
        sorted = true;
        for (size_t i = 0; i < result.bones.size(); ++i) {
            auto& b = result.bones[i];
            auto parent_index = b.parent;
            if (!b.has_parent() || parent_index < i)
                continue;

            b.parent = i;
            std::swap(result.bones[i], result.bones[parent_index]);
            for (auto& w : weight_data) {
                if (w.bone_index == parent_index)
                    w.bone_index = i;
                else if (w.bone_index == i)
                    w.bone_index = parent_index;
            }
            sorted = false;
        }
    }

    // Assign bones and weights to vertices
    auto& vertices = result.vertices;
    size_t* vertex_bone_counts = new size_t[vertices.size()];
    memset(vertex_bone_counts, 0, vertices.size() * sizeof(size_t));

    for (auto w : weight_data) {
        size_t bone_count = vertex_bone_counts[w.vert_index];
        if (bone_count == MAX_BONES_PER_VERTEX)
            continue;

        vertices[w.vert_index].bone_index[bone_count] =
            static_cast<GLuint>(w.bone_index);
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
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex),
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

size_t RiggedMesh::find_bone_index(const char* str) const {
    for (uint i = 0; i < bones.size(); ++i) {
        if (bones[i].name.compare(str) == 0) {
            return i;
        }
    }
    return Bone::INDEX_NOT_FOUND;
}