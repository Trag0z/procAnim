#pragma once
#include "../pch.h"
#include "Collider.h"

class Renderer;
class MouseKeyboardInput;

class LevelEditor;

class Level {
    std::list<BoxCollider> colliders_;

  public:
    void load_from_file(char* path = nullptr);
    void render(const Renderer& renderer) const;

    const std::list<BoxCollider> colliders() const noexcept;

    friend LevelEditor;
};

class LevelEditor {
    Level* level;

    BoxCollider* selected_collider = nullptr;

  public:
    void init(Level* level_);
    bool update(const Renderer& renderer, const MouseKeyboardInput& input);

    void save_level(char* path = nullptr) const;
};