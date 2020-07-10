#pragma once
#include "pch.h"
#include "Level.h"
#include "Collider.h"
#include "../Renderer.h"
#include "../Input.h"

struct BoxColliderSaveFormat {
    glm::vec2 position;
    glm::vec2 half_ext;
};

void Level::load_from_file(char* path) {
    if (path == nullptr) {
        bool success = get_load_path(path, L".level", L"*.level");

        if (!success) {
            // Default ground object
            colliders_.emplace_front(
                glm::vec2(1920.0f / 2.0f, 1080.0f / 2.0f - 400.0f),
                glm::vec2(10000.0f, 10.0f));
            return;
        }
    }

    colliders_.clear();

    // Read data from file
    SDL_RWops* file = SDL_RWFromFile(path, "wb");

    size_t num_colliders;
    SDL_RWread(file, &num_colliders, sizeof(num_colliders), 1);

    BoxColliderSaveFormat* save_data = new BoxColliderSaveFormat[num_colliders];
    SDL_RWread(file, save_data, sizeof(*save_data), num_colliders);

    SDL_RWclose(file);

    // Create list of colliders from data
    for (size_t i = 0; i < num_colliders; ++i) {
        colliders_.emplace_back(save_data[i].position, save_data[i].half_ext);
    }

    delete[] save_data;
}

void Level::render(const Renderer& renderer) const {
    for (auto& coll : colliders_) {
        coll.render(renderer);
    }
}

const std::list<BoxCollider> Level::colliders() const noexcept {
    return colliders_;
}

static constexpr float SCROLL_SPEED = 1.0f;

void LevelEditor::init(Level* level_) { level = level_; }

bool LevelEditor::update(const Renderer& renderer,
                         const MouseKeyboardInput& input) {
    bool keep_open = true;
    auto& colliders = level->colliders_;

    { // UI
        using namespace ImGui;
        Begin("Level Editor", &keep_open);

        if (Button("New collider")) {
            colliders.emplace_front(renderer.camera_position() +
                                        renderer.window_size() * 0.5f,
                                    glm::vec2(1.0f));

            selected_collider = &colliders.front();
        }

        Text("Selected Collider");
        if (selected_collider) {
            bool needs_update = false;
            needs_update |=
                DragFloat2("Position", value_ptr(selected_collider->position),
                           1.0f, 0.0f, 0.0f, "% 6.1f");
            needs_update |=
                DragFloat2("Half ext.", value_ptr(selected_collider->half_ext),
                           0.1f, 0.0f, 0.0f, "% 6.1f");

            if (needs_update) {
                selected_collider->update_vertex_data();
            }
        } else {
            Text("None");
        }

        End();
    }

    if (input.mouse_button_down(1)) {
        glm::vec2 mouse_pos = input.mouse_pos_world();

        for (auto& coll : colliders) {
            if (coll.is_inside_rect(mouse_pos)) {
                dragging_collider = true;
                selected_collider = &coll;
                break;
            }
        }
    }

    if (input.mouse_button_up(1)) {
        dragging_collider = false;
    }

    if (selected_collider) {
        selected_collider->half_ext += input.mouse_wheel_scroll * SCROLL_SPEED;

        if (dragging_collider) {
            selected_collider->position += input.mouse_move();
        }

        selected_collider->update_vertex_data();
    }

    return keep_open;
}

void LevelEditor::save_level(char* path) const {
    if (path == nullptr) {
        bool success = get_save_path(path, L".level", L"*.level", L"level");
        SDL_assert_always(success);
    }

    auto& colliders = level->colliders_;

    size_t num_colliders = colliders.size();
    BoxColliderSaveFormat* save_data = new BoxColliderSaveFormat[num_colliders];

    size_t i = 0;
    for (auto& coll : colliders) {
        save_data[i].position = coll.position;
        save_data[i].half_ext = coll.half_ext;
        ++i;
    }

    SDL_RWops* file = SDL_RWFromFile(path, "wb");
    SDL_RWwrite(file, &num_colliders, sizeof(num_colliders), 1);
    SDL_RWwrite(file, save_data, sizeof(*save_data), num_colliders);
    SDL_RWclose(file);

    delete[] save_data;
}
