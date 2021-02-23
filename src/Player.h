#pragma once
#include <list>
#include "rendering/Texture.h"
#include "rendering/Mesh.h"
#include "Animator.h"
#include "Entity.h"
#include "rendering/Renderer.h"
#include "CollisionDetection.h"
#include "WeaponTrail.h"

class Gamepad;
class ConfigLoader;
struct AABB;

class Player : public Entity {
    Texture texture;
    Mesh body_mesh;
    RiggedMesh rigged_mesh;
    Animator animator;
    const Gamepad* gamepad;

    Circle body_collider_ = {glm::vec2(0.0f), 1.0f};

    glm::vec2 velocity = glm::vec2(0.0f); // In world space

    bool grounded = false;
    bool facing_right = true;

    float hit_cooldown = 0.0f;
    float hitstun_duration = 0.0f;

    float freeze_duration = 0.0f;

    bool can_double_jump = false;
    Direction wall_direction = Direction::NONE;
    float wall_jump_cotyote_time = 0.0f;

    enum State {
        STANDING = 0,
        WALKING = 1,
        FALLING = 2,
        HITSTUN = 3,
        WALL_CLING = 4
    } state = FALLING;

    // These are in world space!
    Segment weapon_collider, last_weapon_collider;

    WeaponTrail weapon_trail;

    // Connfigurable constants
    static float GROUND_HOVER_DISTANCE;
    static float JUMP_FORCE;
    static float DOUBLE_JUMP_FORCE;
    static float GRAVITY;

    static float WALL_JUMP_FORCE;
    static float MAX_WALL_JUMP_COYOTE_TIME;

    static float WALL_SLIDE_SPEED;
    static float MAX_WALL_SLIDE_SPEED;

    static float WALK_ACCELERATION;
    static float MAX_WALK_VELOCITY;

    static float MAX_AIR_ACCELERATION;
    static float MAX_AIR_VELOCITY;

    static float HIT_SPEED_MULTIPLIER;
    static float MAX_HIT_COOLDOWN;
    static float MAX_HIT_TRAIL_ANGLE;
    static float HITSTUN_DURATION_MULTIPLIER;

  public:
    void init(glm::vec3 position, glm::vec3 scale_factor,
              const char* texture_path, const char* mesh_path,
              const Gamepad* pad, const std::list<AABB>& colliders);

    void update(float delta_time, const std::list<AABB>& colliders);

    bool is_facing_right() const noexcept;

    Circle body_collider() const noexcept;

    // Return value signifies if the window should be kept open.
    bool display_debug_ui(size_t player_index);

    // The debug UI needs to access the private members of this class. Letting
    // Game access them this way seems cleaner to me then writing a bunch of
    // getters/setters that are only used in one place.
    friend Game;
    friend ConfigManager;
};