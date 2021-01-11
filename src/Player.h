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

    CircleCollider body_collider_ = {glm::vec2(0.0f), 1.0f};

    Animator animator;
    const Gamepad* gamepad;

    glm::vec2 velocity = glm::vec2(0.0f); // In world space
    bool grounded = false;

    float walk_speed;
    bool facing_right = true;

    static float GROUND_HOVER_DISTANCE;
    static float JUMP_FORCE;
    static float MAX_WALK_SPEED;
    static float MAX_AIR_ACCELERATION;
    static float MAX_AIR_SPEED;
    static float HIT_SPEED_MULTIPLIER;

    // Tese are in world space!
    LineCollider weapon_collider, last_weapon_collider;

    enum State { STANDING, WALKING, FALLING, HITSTUN } state = FALLING;

  public:
    void init(glm::vec3 position, glm::vec3 scale_factor,
              const char* texture_path, const char* mesh_path,
              const Gamepad* pad, const std::list<BoxCollider>& colliders);

    void update(float delta_time, const std::list<BoxCollider>& colliders,
                const MouseKeyboardInput& input);

    bool is_facing_right() const noexcept;

    CircleCollider body_collider() const noexcept;

    // The debug UI needs to access the private members of this class. Letting
    // Game access them this way seems cleaner to me then writing a bunch of
    // getters/setters that are only used in one place.
    friend Game;
};