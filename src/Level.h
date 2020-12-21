#pragma once
#include "pch.h"
#include "Collider.h"

class Renderer;
class MouseKeyboardInput;
struct BoxCollider;

class LevelEditor;

class Level {
    // Linked lists are probably a bad choice for performance here, but they
    // allow for deletion of random elements which the LevelEditor uses
    // frequently.
    std::list<BoxCollider> colliders_;
    std::string opened_path;

    Texture wall_texture;

  public:
    void render(const Renderer& renderer) const;

    const std::list<BoxCollider> colliders() const noexcept;

    const BoxCollider* find_ground_under(glm::vec2 position) const;

    void save_to_file(const char* path) const;
    void load_from_file(const char* path);

    // Let the LevelEditor access our private members so it can manipulate them.
    friend LevelEditor;
};

class LevelEditor {
    Level* level;
    BoxCollider* selected_collider;

    glm::vec2 new_collider_dimensions = glm::vec2(100.0f);

    bool dragging_collider = false;

  public:
    void init(Level* level_);
    bool update(const Renderer& renderer, const MouseKeyboardInput& input);
    void render(const Renderer& renderer);

    void save_to_file(bool new_file_name = false);
    void load_from_file(bool new_file_name = false);
};
