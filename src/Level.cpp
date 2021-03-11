#pragma once
#include "Level.h"
#include "rendering/Renderer.h"
#include "Input.h"
#include "CollisionDetection.h"
#include <imgui/imgui.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtc/type_ptr.hpp>

void Level::render(const Renderer& renderer) const {
    renderer.textured_shader.set_texture(wall_texture);

    for (const auto& coll : colliders_) {
        glm::mat3 model = glm::translate(glm::mat3(1.0f), coll.center);
        model           = glm::scale(model, coll.half_ext);
        renderer.textured_shader.set_model(&model);

        renderer.textured_shader.DEFAULT_VAO.draw(GL_TRIANGLES);
    }
}

const std::list<AABB> Level::colliders() const noexcept {
    return colliders_;
}

const AABB* Level::find_ground_under(glm::vec2 position) const {
    const AABB* candidate = nullptr;
    for (const AABB& coll : colliders_) {
        if (coll.min(0) <= position.x && coll.max(0) >= position.x
            && position.y > coll.max(1)) {

            if (!candidate || coll.max(1) > candidate->max(1)) {
                candidate = &coll;
            }
        }
    }
    return candidate;
}
typedef AABB BoxColliderSaveFormat;

void Level::load_from_file(const char* path) {
    SDL_assert_always(path != nullptr);

    opened_path = std::string(path);

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
        colliders_.emplace_back(save_data[i]);
    }

    delete[] save_data;

    wall_texture.load_from_file("../assets/ground.png");
}

void Level::save_to_file(const char* path) const {
    size_t num_colliders             = colliders_.size();
    BoxColliderSaveFormat* save_data = new BoxColliderSaveFormat[num_colliders];

    size_t i = 0;
    for (auto& coll : colliders_) {
        save_data[i].center   = coll.center;
        save_data[i].half_ext = coll.half_ext;
        ++i;
    }

    SDL_RWops* file = SDL_RWFromFile(path, "wb");
    SDL_RWwrite(file, &num_colliders, sizeof(num_colliders), 1);
    SDL_RWwrite(file, save_data, sizeof(*save_data), num_colliders);
    SDL_RWclose(file);

    delete[] save_data;
}

static constexpr float SCROLL_SPEED = 1.0f;

void LevelEditor::init(Level* level_) {
    level = level_;
}

bool LevelEditor::update(const Renderer& renderer,
                         const MouseKeyboardInput& input) {
    bool keep_open  = true;
    auto& colliders = level->colliders_;

    {  // UI
        using namespace ImGui;
        Begin("Level Editor", &keep_open);
        if (Button("Open...")) { load_from_file(true); }
        SameLine();
        if (Button("Save")) { save_to_file(); }
        SameLine();
        if (Button("Save as...")) { save_to_file(true); }
        Text("Opened file: %s", level->opened_path.c_str());

        NewLine();

        if (Button("New collider")) {
            colliders.emplace_front(
              AABB { renderer.camera_center(), new_collider_dimensions });

            selected_collider = &colliders.front();
        }
        DragFloat2("Dimensions",
                   value_ptr(new_collider_dimensions),
                   1.0f,
                   0.0f,
                   0.0f,
                   "% 6.1f");

        NewLine();
        Text("Selected Collider");
        if (selected_collider) {
            DragFloat2("Position",
                       value_ptr(selected_collider->center),
                       1.0f,
                       0.0f,
                       0.0f,
                       "% 6.1f");
            DragFloat2("Half ext.",
                       value_ptr(selected_collider->half_ext),
                       0.1f,
                       0.0f,
                       0.0f,
                       "% 6.1f");

        } else {
            Text("None");
        }

        End();
    }

    // Select collider
    if (input.mouse_button_down(MouseButton::LEFT)) {
        glm::vec2 mouse_pos = input.mouse_pos_world();

        for (auto& coll : colliders) {
            if (test_point_AABB(mouse_pos, coll)) {
                dragging_collider = true;
                selected_collider = &coll;
                break;
            }
        }
    }
    if (input.mouse_button_up(MouseButton::LEFT)) { dragging_collider = false; }

    // Deselect collider
    if (input.mouse_button_down(MouseButton::RIGHT)) {
        selected_collider = nullptr;
    }

    if (selected_collider) {

        selected_collider->half_ext += input.mouse_wheel_scroll * SCROLL_SPEED;

        if (dragging_collider) {
            selected_collider->center += input.mouse_move_world();
        }

        // Remove at the end so the other operations can happen before
        if (input.key_down(SDL_SCANCODE_DELETE)) {
            colliders.remove(*selected_collider);
            selected_collider = nullptr;
            dragging_collider = false;
        }
    }

    return keep_open;
}

void LevelEditor::render(const Renderer& renderer) {
    if (selected_collider) {
        renderer.debug_shader.set_color(Color::LIGHT_BLUE);
        glm::mat3 model = selected_collider->calculate_model_matrix();
        renderer.debug_shader.set_model(&model);
        renderer.debug_shader.SQUARE_VAO.draw(GL_LINE_LOOP);
    }
}

void LevelEditor::save_to_file(bool new_file_name) {
    if (new_file_name || !level->opened_path.empty()) {
        bool success =
          get_save_path(level->opened_path, L".level", L"*.level", L"level");
        SDL_assert_always(success);
    }

    level->save_to_file(level->opened_path.c_str());
}

void LevelEditor::load_from_file(bool new_file_name) {
    std::string new_path;
    if (new_file_name || !level->opened_path.empty()) {
        if (!get_load_path(new_path, L".level", L"*.level")) { return; }
    }

    SDL_assert(level);
    level->load_from_file(new_path.c_str());
}