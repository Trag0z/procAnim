#pragma once
#include "pch.h"
#include "Texture.h"
#include "Mesh.h"
#include "Collider.h"
#include "Animator.h"
#include "Entity.h"
#include "Renderer.h"

class Gamepad;
struct GameConfig;

class Player : Entity {
    Texture tex;
    RiggedMesh rigged_mesh;
    WalkAnimator animator;
    Gamepad* gamepad_input; // currently unused

    bool grounded;
    float walking_speed = 0.03f;
    const float gravity = 4.0f;
    bool facing_right = true;
    AnimState anim_state;

    bool spline_edit_mode = false;

  public:
    void init(glm::vec3 position, glm::vec3 scale_factor,
              const char* texture_path, const char* mesh_path,
              Gamepad* gamepad);

    void update(float delta_time, const BoxCollider& ground,
                const MouseKeyboardInput& mkb);

    void render(const Renderer& renderer);

    // The general UI displays the player's private members, so it needs to
    // access them. This feels cleaner than writing a bunch of getters.
    friend void update_gui(SDL_Window* window, Renderer& renderer,
                           GameConfig& game_config, Player& player);
};