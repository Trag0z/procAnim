#pragma once
#include <list>
#include "Collider.h"
#include "rendering/Texture.h"

class Renderer;
class MouseKeyboardInput;
struct AABB;

class LevelEditor;

class Level {
  public:
    static const size_t NUM_GOALS = 2;

    // Linked lists are probably a bad choice for performance here, but they
    // allow for deletion of random elements which the LevelEditor uses
    // frequently.
    std::list<AABB> colliders;
    Texture wall_texture;

    struct {
        std::list<AABB> colliders;
        Texture texture;
    } goals[NUM_GOALS];

    std::string opened_path;

    void render(const Renderer& renderer) const;

    const AABB* find_ground_under(glm::vec2 position) const;

    void save_to_file(const char* path) const;
    void load_from_file(const char* path);

    // Let the LevelEditor access our private members so it can manipulate them.
    friend LevelEditor;
};

class LevelEditor {
    Level* level;
    AABB* selected_collider;

    glm::vec2 new_collider_dimensions = glm::vec2(100.0f);

    bool dragging_collider = false;

    int selected_team = 0;

  public:
    void init(Level* level_);
    bool update(const Renderer& renderer, const MouseKeyboardInput& input);
    void render(const Renderer& renderer);

    void save_to_file(bool new_file_name = false);
    void load_from_file(bool new_file_name = false);
};
