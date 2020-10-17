#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Util.h"

glm::mat3 Bone::transform() const {
    // Recurse until there's no parent
    if (parent) {
        glm::mat3 this_transform = bind_pose_transform *
                                   glm::rotate(glm::mat3(1.0f), rotation) *
                                   inverse_bind_pose_transform;
        return parent->transform() * this_transform;
    }

    return bind_pose_transform * glm::rotate(glm::mat3(1.0f), rotation) *
           inverse_bind_pose_transform;
}

glm::vec2 Bone::head() const { return transform() * bind_pose_transform[2]; }

void Mesh::load_from_file(const char* file) {
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
    Mesh result;

    std::vector<RiggedShader::Vertex> shader_vertices;
    shader_vertices.reserve(mesh_data.mNumVertices);

    aiVector3D* vertex_data = mesh_data.mVertices;
    aiVector3D* uv_coords = mesh_data.mTextureCoords[0];
    SDL_assert(uv_coords);

    for (size_t i = 0; i < mesh_data.mNumVertices; ++i) {
        shader_vertices.push_back(
            {glm::vec2(vertex_data[i].x, vertex_data[i].y),
             {uv_coords[i].x, 1.0f - uv_coords[i].y},
             {0, 0},
             {0.0f, 0.0f}}); // Or 0.5f, 0.5f?
    }

    std::vector<GLuint> indices;
    indices.reserve(static_cast<size_t>(mesh_data.mNumFaces) * 3);
    for (size_t i = 0; i < mesh_data.mNumFaces; ++i) {
        aiFace& face = mesh_data.mFaces[i];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    // Parse bones and sort weights/indices into vertex data
    struct WeightData {
        size_t vert_index, bone_index;
        float weight;
    };
    std::vector<WeightData> weight_data;

    // Create bones and populate weight_data
    bones.reserve(mesh_data.mNumBones);
    for (uint n_bone = 0; n_bone < mesh_data.mNumBones; ++n_bone) {
        Bone bone;

        aiBone& ai_bone = *mesh_data.mBones[n_bone];
        bone.name = ai_bone.mName.C_Str();

        auto& matrix = ai_bone.mOffsetMatrix;
        bone.inverse_bind_pose_transform =
            glm::mat3(matrix.a1, matrix.b1, matrix.d1, matrix.a2, matrix.b2,
                      matrix.d2, matrix.a4, matrix.b4, matrix.d4);
        bone.bind_pose_transform =
            glm::inverse(bone.inverse_bind_pose_transform);

        for (uint n_weight = 0; n_weight < ai_bone.mNumWeights; ++n_weight) {
            weight_data.push_back({ai_bone.mWeights[n_weight].mVertexId, n_bone,
                                   ai_bone.mWeights[n_weight].mWeight});
        }

        bones.push_back(bone);
    }

    // Find bone parents, calculate length and tail positions
    aiNode* root = scene->mRootNode;

    for (auto& b : bones) {
        aiNode* node = root->FindNode(b.name.c_str());
        b.parent = find_bone(node->mParent->mName.C_Str());

        if (node->mNumChildren > 0) {
            auto& child_transform = node->mChildren[0]->mTransformation;
            b.tail = glm::vec2(child_transform.a4, child_transform.b4);

            b.length = glm::length(b.tail);
        }
    }

    // Assign bones and weights to shader_vertices
    size_t* vertex_bone_counts = new size_t[shader_vertices.size()];
    memset(vertex_bone_counts, 0, shader_vertices.size() * sizeof(size_t));

    for (auto this_weight : weight_data) {
        size_t bone_count = vertex_bone_counts[this_weight.vert_index];
        if (bone_count == RiggedShader::MAX_BONES_PER_VERTEX)
            continue;

        shader_vertices[this_weight.vert_index].bone_indices[bone_count] =
            static_cast<GLuint>(this_weight.bone_index);
        shader_vertices[this_weight.vert_index].bone_weights[bone_count] =
            this_weight.weight;
        ++bone_count;
    }
    delete[] vertex_bone_counts;

    vao.init(indices.data(), static_cast<GLuint>(indices.size()),
             shader_vertices.data(),
             static_cast<GLuint>(shader_vertices.size()), GL_STATIC_DRAW);

    // Create and upload bone render data to GPU
    const GLuint num_bone_vertices = mesh_data.mNumBones * 2;
    std::vector<GLuint> bone_indices;
    bone_indices.reserve(num_bone_vertices);
    for (GLuint i = 0; i < num_bone_vertices; ++i) {
        bone_indices.push_back(i);
    }

    std::vector<BoneShader::Vertex> bone_vertices;
    bone_vertices.reserve(mesh_data.mNumBones * 2);
    for (size_t i = 0; i < bones.size(); ++i) {
        bone_vertices.push_back({bones[i].bind_pose_transform[2]});
        bone_vertices.push_back(
            {bones[i].bind_pose_transform * glm::vec3(bones[i].tail, 1.0f)});
    }

    bones_vao.init(bone_indices.data(), num_bone_vertices, bone_vertices.data(),
                   num_bone_vertices, GL_STATIC_DRAW);
}

Bone* Mesh::find_bone(const char* str) {
    for (uint i = 0; i < bones.size(); ++i) {
        if (bones[i].name.compare(str) == 0) {
            return &bones[i];
        }
    }
    return nullptr;
}