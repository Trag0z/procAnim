#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Util.h"

void RiggedMesh::load_from_file(const char* file) {
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

    vertices.reserve(mesh_data.mNumVertices);
    shader_vertices.reserve(mesh_data.mNumVertices);

    aiVector3D* vertData = mesh_data.mVertices;
    aiVector3D* uvCoords = mesh_data.mTextureCoords[0];
    SDL_assert(uvCoords);

    for (size_t i = 0; i < mesh_data.mNumVertices; ++i) {
        vertices.push_back({{vertData[i].x, vertData[i].y, 0.0f},
                            {uvCoords[i].x, 1.0f - uvCoords[i].y},
                            {0, 0},
                            {0.0f, 0.0f}}); // Or 0.5f, 0.5f?
        shader_vertices.push_back({});
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

    // TODO: Maybe remove later
    auto convert_to_column_major = [](glm::mat4& out, aiMatrix4x4& in) {
        memcpy(&out, &in, sizeof(float) * 4 * 4);
        out = glm::transpose(out);
    };

    // Create bones and populate weight_data
    bones.reserve(mesh_data.mNumBones);
    bones_shader_vertices.reserve(mesh_data.mNumBones * 2);
    for (uint i = 0; i < mesh_data.mNumBones; ++i) {
        Bone b;

        aiBone& ai_bone = *mesh_data.mBones[i];
        b.name = ai_bone.mName.C_Str();
        convert_to_column_major(b.inverse_bind_pose_transform,
                                ai_bone.mOffsetMatrix);
        b.length = 0.0f;

        for (uint j = 0; j < ai_bone.mNumWeights; ++j) {
            weight_data.push_back({ai_bone.mWeights[j].mVertexId, i,
                                   ai_bone.mWeights[j].mWeight});
        }

        bones.push_back(b);
        bones_shader_vertices.push_back({glm::vec4()});
        bones_shader_vertices.push_back({glm::vec4()});
    }

    // Find bone parents, calculate length and head/tail positions
    aiNode* root = scene->mRootNode;

    for (auto& b : bones) {
        aiNode* node = root->FindNode(b.name.c_str());
        b.parent = find_bone_index(node->mParent->mName.C_Str());

        if (node->mNumChildren > 0) {
            auto& child_transform = node->mChildren[0]->mTransformation;
            b.tail = glm::vec4(child_transform.a4, child_transform.b4,
                               child_transform.c4, 1.0f);

            b.bind_pose_transform = inverse(b.inverse_bind_pose_transform);
            b.length = glm::length(b.tail);
        }
    }

    // Sort array so parents are always before children
    // NOTE: Does not take into account multiple children on the same bone,
    // wrong if there are longer chains of inheritence
    // TODO: clean this up, probably change the whole function
    bool sorted = false;
    while (!sorted) {
        sorted = true;
        for (size_t i = 0; i < bones.size(); ++i) {
            auto& b = bones[i];
            auto parent_index = b.parent;
            if (!b.has_parent() || parent_index < i)
                continue;

            b.parent = i;
            std::swap(bones[i], bones[parent_index]);
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

    // Init LimbAnimators
    animators.reserve(4);
    animators.push_back({&bones[find_bone_index("Arm_L_1")],
                         &bones[find_bone_index("Arm_L_2")]});
    animators.push_back({&bones[find_bone_index("Arm_R_1")],
                         &bones[find_bone_index("Arm_R_2")]});
    animators.push_back({&bones[find_bone_index("Leg_L_1")],
                         &bones[find_bone_index("Leg_L_2")]});
    animators.push_back({&bones[find_bone_index("Leg_R_1")],
                         &bones[find_bone_index("Leg_R_2")]});

    vao.init(indices.data(), static_cast<GLuint>(indices.size()), NULL,
             static_cast<GLuint>(vertices.size()));

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

    // Create and upload bone render data to GPU
    size_t num_bone_indices = bones_shader_vertices.size();
    GLuint* bone_indices = new GLuint[num_bone_indices];
    for (size_t i = 0; i < num_bone_indices; ++i) {
        bone_indices[i] = static_cast<GLuint>(i);
    }

    bones_vao.init(bone_indices, static_cast<GLuint>(num_bone_indices), NULL,
                   static_cast<GLuint>(bones_shader_vertices.size()));
}

size_t RiggedMesh::find_bone_index(const char* str) const {
    for (uint i = 0; i < bones.size(); ++i) {
        if (bones[i].name.compare(str) == 0) {
            return i;
        }
    }
    return Bone::INDEX_NOT_FOUND;
}