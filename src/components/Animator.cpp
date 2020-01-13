#include "pch.h"
#include "Animator.h"

Bone::Bone(aiBone& b) : name(b.mName.C_Str()) {
    memcpy(&offsetMatrix, &b.mOffsetMatrix, sizeof(float) * 4 * 4);
    offsetMatrix = glm::transpose(offsetMatrix);
    for (size_t i = 0; i < b.mNumWeights; ++i) {
        weights.push_back({b.mWeights[i].mVertexId, b.mWeights[i].mWeight});
    }
}