#pragma once
#include "../pch.h"
#include "Collider.h"

class Renderer;
class MouseKeyboardInput;

class LevelEditor;

class Level {
    std::list<BoxCollider> colliders_;
    std::string opened_path;

  public:
    void load_from_file(const char* path);
    void render(const Renderer& renderer) const;

    const std::list<BoxCollider> colliders() const noexcept;

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