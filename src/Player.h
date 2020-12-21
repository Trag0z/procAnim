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
    Mesh body_mesh;
    RiggedMesh rigged_mesh;

    CircleCollider body_collider = {glm::vec2(0.0f), 1.0f};

    Animator animator;
    const Gamepad* gamepad;

    glm::vec2 velocity = glm::vec2(0.0f); // In world space
    bool grounded = false;

    float walk_speed;
    bool facing_right = true;

    float distance_to_ground = 160.0f;
    float jump_force = 30.0f;
    float max_walk_speed = 10.0f;
    float gravity = 2.0f;

    enum State { STANDING, WALKING, FALLING } state = FALLING;

  public:
    void init(glm::vec3 position, glm::vec3 scale_factor,
              const char* texture_path, const char* mesh_path,
              const Gamepad* pad, const std::list<BoxCollider>& colliders);

    void update(float delta_time, const std::list<BoxCollider>& colliders,
                const MouseKeyboardInput& input);

    void render(const Renderer& renderer);

    bool is_facing_right() const noexcept;

    // The debug UI needs to access the private members of this class. Letting
    // Game access them this way seems cleaner to me then writing a bunch of
    // getters/setters that are only used in one place.
    friend Game;
};