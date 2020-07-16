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

void Level::load_from_file(const char* path) {
    SDL_assert_always(path);
    SDL_assert_always(path != opened_path);

    if (opened_path) {
        delete[] opened_path;
    }
    size_t length = strnlen_s(path, 256) + 1;
    opened_path = new char[length];
    strcpy_s(opened_path, length, path);

    colliders_.clear();

    // Read data from file
    SDL_RWops* file = SDL_RWFromFile(path, "rb");

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
    renderer.textured_shader.set_texture(BoxCollider::TEXTURE);
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
        if (Button("Open...")) {
            load_from_file(true);
        }
        SameLine();
        if (Button("Save")) {
            save_to_file();
        }
        SameLine();
        if (Button("Save as...")) {
            save_to_file(true);
        }
        Text("Opened file: %s", level->opened_path);

        NewLine();

        if (Button("New collider")) {
            colliders.emplace_front(renderer.camera_position() +
                                        renderer.window_size() * 0.5f,
                                    new_collider_dimensions);

            selected_collider = &colliders.front();
        }
        DragFloat2("Dimensions", value_ptr(new_collider_dimensions), 1.0f, 0.0f,
                   0.0f, "% 6.1f");

        NewLine();
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
                selected_collider->update_model_matrix();
            }
        } else {
            Text("None");
        }

        End();
    }

    // Select collider
    if (input.mouse_button_down(MouseButton::LEFT)) {
        glm::vec2 mouse_pos = input.mouse_pos_world();

        for (auto& coll : colliders) {
            if (coll.is_inside_rect(mouse_pos)) {
                dragging_collider = true;
                selected_collider = &coll;
                break;
            }
        }
    }
    if (input.mouse_button_up(MouseButton::LEFT)) {
        dragging_collider = false;
    }

    // Deselect collider
    if (input.mouse_button_down(MouseButton::RIGHT)) {
        selected_collider = nullptr;
    }

    if (selected_collider) {
        selected_collider->half_ext += input.mouse_wheel_scroll * SCROLL_SPEED;

        if (dragging_collider) {
            selected_collider->position += input.mouse_move();
        }

        selected_collider->update_model_matrix();
    }

    return keep_open;
}

void LevelEditor::render(const Renderer& renderer) {
    if (selected_collider) {
        renderer.debug_shader.set_color(&Color::LIGHT_BLUE);
        renderer.debug_shader.set_model(&selected_collider->model);
        renderer.debug_shader.DEFAULT_VAO.draw(GL_LINE_LOOP);
    }
}

void LevelEditor::save_to_file(bool new_file_name) {
    if (new_file_name || level->opened_path == nullptr) {
        bool success =
            get_save_path(level->opened_path, L".level", L"*.level", L"level");
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

    SDL_RWops* file = SDL_RWFromFile(level->opened_path, "wb");
    SDL_RWwrite(file, &num_colliders, sizeof(num_colliders), 1);
    SDL_RWwrite(file, save_data, sizeof(*save_data), num_colliders);
    SDL_RWclose(file);

    delete[] save_data;
}

void LevelEditor::load_from_file(bool new_file_name) {
    char* new_path = nullptr;
    if (new_file_name || level->opened_path == nullptr) {
        if (!get_load_path(new_path, L".level", L"*.level")) {
            return;
        }
    }

    SDL_assert(level);
    level->load_from_file(new_path);
}