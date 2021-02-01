#pragma once
#include "pch.h"
#include "rendering/Texture.h"
#include "rendering/Mesh.h"
#include "Animator.h"
#include "Entity.h"
#include "rendering/Renderer.h"

class Gamepad;
class ConfigLoader;
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
    float time_since_last_hit = 0.0f;
    float hitstun_duration = 0.0f;

    static float GROUND_HOVER_DISTANCE;
    static float JUMP_FORCE;
    static float MAX_WALK_SPEED;
    static float MAX_AIR_ACCELERATION;
    static float MAX_AIR_SPEED;
    static float HIT_SPEED_MULTIPLIER;
    static float HIT_COOLDOWN;
    static float HITSTUN_DURATION_MULTIPLIER;

    // These are in world space!
    LineCollider weapon_collider, last_weapon_collider;

    enum State {
        STANDING = 0,
        WALKING = 1,
        FALLING = 2,
        HITSTUN = 3
    } state = FALLING;

  public:
    void init(glm::vec3 position, glm::vec3 scale_factor,
              const char* texture_path, const char* mesh_path,
              const Gamepad* pad, const std::list<BoxCollider>& colliders);

    void update(float delta_time, const std::list<BoxCollider>& colliders,
                const MouseKeyboardInput& input);

    bool is_facing_right() const noexcept;

    CircleCollider body_collider() const noexcept;

    // Return value signifies if the window should be kept open.
    bool display_debug_ui_window(size_t player_index);

    // The debug UI needs to access the private members of this class. Letting
    // Game access them this way seems cleaner to me then writing a bunch of
    // getters/setters that are only used in one place.
    friend Game;
    friend ConfigManager;
};