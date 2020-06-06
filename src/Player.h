#pragma once
#include "pch.h"
#include "Texture.h"
#include "Mesh.h"

class Gamepad;

struct Player {
    glm::vec2 pos;
    glm::vec3 scale;
    Texture tex;
    RiggedMesh rigged_mesh;
    Gamepad* gamepad_input; // currently unused

    glm::mat4 model;

    enum AnimState { STANDING, WALKING } anim_state;

    enum WalkState { LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN } walk_state;

    bool grounded;
    float walking_speed = 0.1f;
    const float gravity = 2.0f;
    bool facing_right = true;

    void init(glm::vec2 position, glm::vec3 scale_factor,
              const char* texture_path, const char* mesh_path,
              Gamepad* gamepad);
};