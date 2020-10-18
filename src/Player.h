#pragma once
#include "pch.h"
#include "Texture.h"
#include "Mesh.h"
#include "Animator.h"
#include "Entity.h"
#include "Renderer.h"

class Gamepad;
struct BoxCollider;

class Player : public Entity {
    Texture texture;
    Mesh mesh;
    Animator animator;
    const Gamepad* gamepad;

    float walking_speed;
    bool facing_right = true;

  public:
    void init(glm::vec3 position, glm::vec3 scale_factor,
              const char* texture_path, const char* mesh_path,
              const Gamepad* pad, const std::list<BoxCollider>& colliders);

    void update(float delta_time, const std::list<BoxCollider>& colliders,
                const MouseKeyboardInput& input);

    void render(const Renderer& renderer);

    bool is_facing_right() const noexcept;

    // The general UI displays the player's private members, so it needs to
    // access them. This seems cleaner to me than writing a bunch of
    // getters/setters.
    friend Game;
};