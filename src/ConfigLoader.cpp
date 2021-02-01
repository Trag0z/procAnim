#pragma once
#include "pch.h"
#include "ConfigLoader.h"
#include "Game.h"

void ConfigLoader::init(GameConfig& game_config, Renderer& renderer) {
    // GameConfig
    items.emplace("window_position", &game_config.window_position);
    items.emplace("gravity", &game_config.gravity);
    items.emplace("use_const_delta_time", &game_config.use_const_delta_time);
    items.emplace("step_mode", &game_config.step_mode);
    items.emplace("speed", &game_config.speed);

    // Renderer
    items.emplace("window_size", &renderer.window_size_);
    items.emplace("camera_center", &renderer.camera_center_);
    items.emplace("zoom_factor", &renderer.zoom_factor_);
    items.emplace("draw_body", &renderer.draw_body);
    items.emplace("draw_limbs", &renderer.draw_limbs);
    items.emplace("draw_bones", &renderer.draw_bones);
    items.emplace("draw_wireframes", &renderer.draw_wireframes);
    items.emplace("draw_colliders", &renderer.draw_colliders);
    items.emplace("draw_leg_splines", &renderer.draw_leg_splines);

    // Player
    items.emplace("ground_hover_distance", &Player::GROUND_HOVER_DISTANCE);
    items.emplace("jump_force", &Player::JUMP_FORCE);
    items.emplace("max_walk_speed", &Player::MAX_WALK_SPEED);
    items.emplace("max_air_acceleration", &Player::MAX_AIR_ACCELERATION);
    items.emplace("max_air_speed", &Player::MAX_AIR_SPEED);
    items.emplace("hit_speed_multiplier", &Player::HIT_SPEED_MULTIPLIER);
    items.emplace("hit_cooldown", &Player::HIT_COOLDOWN);
    items.emplace("hitstun_duration_multiplier",
                  &Player::HITSTUN_DURATION_MULTIPLIER);

    // Gamepad
    items.emplace("stick_deadzone_in", &Gamepad::STICK_DEADZONE_IN);
    items.emplace("stick_deadzone_out", &Gamepad::STICK_DEADZONE_OUT);
}

void ConfigLoader::load_config(const char* path) {
    if (path) {
        SDL_assert(save_path.empty());
        save_path = std::string(path);
    } else {
        SDL_assert(!save_path.empty());
    }

    std::ifstream file_stream(save_path);
    SDL_assert(file_stream.is_open());

    for (std::string line; std::getline(file_stream, line);) {
        if (line.empty() || line.at(0) == '#') {
            continue;
        }

        std::stringstream stream{line};

        std::string word;
        std::getline(stream, word, ' ');

        std::visit(ParseVisitor{stream}, items[word]);
    }
}

void ConfigLoader::save_config(const char* path) {
    if (path) {
        SDL_assert(save_path.empty());
        save_path = std::string(path);
    } else {
        SDL_assert(!save_path.empty());
    }
}

bool ConfigLoader::display_ui_window() {
    using namespace ImGui;
    bool keep_open = true;

    Begin("Config Editor", &keep_open);
    if (Button("Save")) {
        save_config();
    }
    SameLine();
    if (Button("Load")) {
        load_config();
    }

    PushItemWidth(150);
    for (auto item : items) {
        std::visit(UIVisitor{item.first}, item.second);
    }
    PopItemWidth();
    End();

    return keep_open;
}

//                    Visitor Object Methods                        //
void ConfigLoader::ParseVisitor::operator()(bool* val) {
    std::string word;
    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    *val = std::stoi(word);
    SDL_assert(*val == 0 || *val == 1);
}
void ConfigLoader::ParseVisitor::operator()(float* val) {
    std::string word;
    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    *val = std::stof(word);
}
void ConfigLoader::ParseVisitor::operator()(s32* val) {
    std::string word;
    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    *val = static_cast<s32>(std::stoi(word));
}
void ConfigLoader::ParseVisitor::operator()(glm::vec2* val) {
    std::string word;

    std::getline(stream, word, ',');
    SDL_assert(word.length() >= 0);
    val->x = std::stof(word);

    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    val->y = std::stof(word);
}
void ConfigLoader::ParseVisitor::operator()(glm::ivec2* val) {
    std::string word;

    std::getline(stream, word, ',');
    SDL_assert(word.length() >= 0);
    val->x = static_cast<glm::i32>(std::stod(word));

    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    val->y = static_cast<glm::i32>(std::stod(word));
}

void ConfigLoader::UIVisitor::operator()(bool* val) {
    ImGui::Checkbox(item_name.c_str(), val);
}
void ConfigLoader::UIVisitor::operator()(float* val) {
    ImGui::InputFloat(item_name.c_str(), val, 1.0f, 10.0f, 2);
}
void ConfigLoader::UIVisitor::operator()(s32* val) {
    const s32 step = 1;
    ImGui::InputScalar(item_name.c_str(), ImGuiDataType_U32, val, &step);
}
void ConfigLoader::UIVisitor::operator()(glm::vec2* val) {
    ImGui::DragFloat2(item_name.c_str(), value_ptr(*val));
}
void ConfigLoader::UIVisitor::operator()(glm::ivec2* val) {
    ImGui::DragInt2(item_name.c_str(), value_ptr(*val));
}