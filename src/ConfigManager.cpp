#pragma once
#include "ConfigManager.h"
#include "Game.h"
#include <fstream>
#include <sstream>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

void ConfigManager::init(GameConfig& game_config, Renderer& renderer) {
    property_map items;

    // GameConfig
    items.emplace("window_position", &game_config.window_position);
    items.emplace("speed", &game_config.speed);
    items.emplace("use_const_delta_time", &game_config.use_const_delta_time);
    items.emplace("step_mode", &game_config.step_mode);
    items.emplace("hit_screen_shake_intensity",
                  &game_config.hit_screen_shake_intensity);
    items.emplace("hit_screen_shake_duration",
                  &game_config.hit_screen_shake_duration);
    items.emplace("hit_screen_shake_speed",
                  &game_config.hit_screen_shake_speed);
    objects.emplace("GameConfig", std::move(items));

    // Renderer
    items.clear();
    items.emplace("window_size", &renderer.window_size_);
    items.emplace("camera_center", &renderer.camera_center_);
    items.emplace("zoom_factor", &renderer.zoom_factor_);
    items.emplace("draw_body", &renderer.draw_body);
    items.emplace("draw_limbs", &renderer.draw_limbs);
    items.emplace("draw_bones", &renderer.draw_bones);
    items.emplace("draw_wireframes", &renderer.draw_wireframes);
    items.emplace("draw_colliders", &renderer.draw_colliders);
    items.emplace("draw_leg_splines", &renderer.draw_leg_splines);
    objects.emplace("Renderer", std::move(items));

    // Player
    items.clear();
    items.emplace("ground_hover_distance", &Player::GROUND_HOVER_DISTANCE);
    items.emplace("jump_force", &Player::JUMP_FORCE);
    items.emplace("double_jump_force", &Player::DOUBLE_JUMP_FORCE);
    items.emplace("gravity", &Player::GRAVITY);

    items.emplace("wall_jump_force", &Player::WALL_JUMP_FORCE);
    items.emplace("max_wall_jump_coyote_time",
                  &Player::MAX_WALL_JUMP_COYOTE_TIME);

    items.emplace("wall_slide_speed", &Player::WALL_SLIDE_SPEED);
    items.emplace("max_wall_slide_speed", &Player::MAX_WALL_SLIDE_SPEED);

    items.emplace("max_walk_acceleration", &Player::WALK_ACCELERATION);
    items.emplace("max_walk_velocity", &Player::MAX_WALK_VELOCITY);

    items.emplace("max_air_acceleration", &Player::MAX_AIR_ACCELERATION);
    items.emplace("max_air_velocity", &Player::MAX_AIR_VELOCITY);

    items.emplace("hit_speed_multiplier", &Player::HIT_SPEED_MULTIPLIER);
    items.emplace("hit_cooldown", &Player::MAX_HIT_COOLDOWN);
    items.emplace("hitstun_duration_multiplier",
                  &Player::HITSTUN_DURATION_MULTIPLIER);
    objects.emplace("Player", std::move(items));

    // Ball
    items.clear();
    items.emplace("damping_factor", &Ball::REBOUND);
    items.emplace("radius", &Ball::RADIUS);
    items.emplace("rolling_friction", &Ball::ROLLING_FRICTION);
    items.emplace("gravity", &Ball::GRAVITY);
    items.emplace("rolling_rotation_speed", &Ball::ROLLING_ROTATION_SPEED);
    objects.emplace("Ball", std::move(items));

    // Gamepad
    items.clear();
    items.emplace("stick_deadzone_in", &Gamepad::STICK_DEADZONE_IN);
    items.emplace("stick_deadzone_out", &Gamepad::STICK_DEADZONE_OUT);
    objects.emplace("Gamepad", std::move(items));
}

void ConfigManager::load_config(const char* path) {
    if (path) {
        SDL_assert(save_path.empty());
        save_path = std::string(path);
    } else {
        SDL_assert(!save_path.empty());
    }

    std::ifstream file_stream(save_path);
    SDL_assert(file_stream.is_open());

    property_map* current_property = nullptr;
    for (std::string line; std::getline(file_stream, line);) {
        if (line.empty()) {
            continue;
        }
        std::stringstream stream{line};

        std::string word;
        std::getline(stream, word, ' ');

        if (line.at(0) == '#') {
            std::getline(stream, word);
            current_property = &objects[word];
            continue;
        }

        SDL_assert(current_property);
        std::visit(ParseVisitor{stream}, (*current_property)[word]);
    }

    file_stream.close();
}

void ConfigManager::save_config() {
    SDL_assert(!save_path.empty());
    std::stringstream stream;

    const size_t buf_size = 1024;
    char buf[buf_size];

    SaveVisitor::write_pos = buf;
    SaveVisitor::buf_end = buf + buf_size;

    for (const auto& obj : objects) {
        SaveVisitor::write_comment(obj.first);

        for (const auto& property : obj.second) {
            std::visit(SaveVisitor{property.first}, property.second);
        }
    }

    SDL_RWops* file = SDL_RWFromFile(save_path.c_str(), "w");

    size_t num_bytes_to_write = SaveVisitor::write_pos - buf;
    size_t num_bytes_written =
        SDL_RWwrite(file, buf, sizeof(char), num_bytes_to_write);

    SDL_assert(num_bytes_written == num_bytes_to_write);

    SDL_RWclose(file);
}

bool ConfigManager::display_ui_window() {
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
    for (auto& obj : objects) {
        if (CollapsingHeader(obj.first.c_str())) {

            for (auto& property : obj.second) {
                std::visit(UIVisitor{property.first}, property.second);
            }
        }
    }
    PopItemWidth();
    End();

    return keep_open;
}

//                    Visitor Object Methods                        //
void ConfigManager::ParseVisitor::operator()(bool* val) {
    std::string word;
    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    *val = std::stoi(word);
    SDL_assert(*val == 0 || *val == 1);
}
void ConfigManager::ParseVisitor::operator()(float* val) {
    std::string word;
    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    *val = std::stof(word);
}
void ConfigManager::ParseVisitor::operator()(s16* val) {
    std::string word;
    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    *val = static_cast<s16>(std::stoi(word));
}
void ConfigManager::ParseVisitor::operator()(s32* val) {
    std::string word;
    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    *val = static_cast<s32>(std::stoi(word));
}
void ConfigManager::ParseVisitor::operator()(glm::vec2* val) {
    std::string word;

    std::getline(stream, word, ',');
    SDL_assert(word.length() >= 0);
    val->x = std::stof(word);

    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    val->y = std::stof(word);
}
void ConfigManager::ParseVisitor::operator()(glm::ivec2* val) {
    std::string word;

    std::getline(stream, word, ',');
    SDL_assert(word.length() >= 0);
    val->x = static_cast<glm::i32>(std::stod(word));

    std::getline(stream, word);
    SDL_assert(word.length() >= 0);
    val->y = static_cast<glm::i32>(std::stod(word));
}

char* ConfigManager::SaveVisitor::write_pos = nullptr;
const char* ConfigManager::SaveVisitor::buf_end = nullptr;

void ConfigManager::SaveVisitor::operator()(bool* val) {
    int num_written = sprintf_s(write_pos, buf_end - write_pos, "%s %d\n",
                                item_name.c_str(), static_cast<int>(*val));
    SDL_assert(num_written != -1);
    write_pos += num_written;
}
void ConfigManager::SaveVisitor::operator()(float* val) {
    int num_written = sprintf_s(write_pos, buf_end - write_pos, "%s %f\n",
                                item_name.c_str(), *val);
    SDL_assert(num_written != -1);
    write_pos += num_written;
}
void ConfigManager::SaveVisitor::operator()(s16* val) {
    int num_written = sprintf_s(write_pos, buf_end - write_pos, "%s %d\n",
                                item_name.c_str(), static_cast<int>(*val));
    SDL_assert(num_written != -1);
    write_pos += num_written;
}
void ConfigManager::SaveVisitor::operator()(s32* val) {
    int num_written = sprintf_s(write_pos, buf_end - write_pos, "%s %d\n",
                                item_name.c_str(), static_cast<int>(*val));
    SDL_assert(num_written != -1);
    write_pos += num_written;
}
void ConfigManager::SaveVisitor::operator()(glm::vec2* val) {
    int num_written =
        sprintf_s(write_pos, buf_end - write_pos, "%s %.2f,%.2f\n",
                  item_name.c_str(), val->x, val->y);
    SDL_assert(num_written != -1);
    write_pos += num_written;
}
void ConfigManager::SaveVisitor::operator()(glm::ivec2* val) {
    int num_written = sprintf_s(write_pos, buf_end - write_pos, "%s %d,%d\n",
                                item_name.c_str(), val->x, val->y);
    SDL_assert(num_written != -1);
    write_pos += num_written;
}
void ConfigManager::SaveVisitor::write_comment(const std::string& str) {
    int num_written =
        sprintf_s(write_pos, buf_end - write_pos, "# %s\n", str.c_str());
    SDL_assert(num_written != -1);
    write_pos += num_written;
}

void ConfigManager::UIVisitor::operator()(bool* val) {
    ImGui::Checkbox(item_name.c_str(), val);
}
void ConfigManager::UIVisitor::operator()(float* val) {
    ImGui::InputFloat(item_name.c_str(), val, 1.0f, 10.0f, 2);
}
void ConfigManager::UIVisitor::operator()(s16* val) {
    const s32 step = 1;
    ImGui::InputScalar(item_name.c_str(), ImGuiDataType_U16, val, &step);
}
void ConfigManager::UIVisitor::operator()(s32* val) {
    const s32 step = 1;
    ImGui::InputScalar(item_name.c_str(), ImGuiDataType_U32, val, &step);
}
void ConfigManager::UIVisitor::operator()(glm::vec2* val) {
    ImGui::DragFloat2(item_name.c_str(), value_ptr(*val));
}
void ConfigManager::UIVisitor::operator()(glm::ivec2* val) {
    ImGui::DragInt2(item_name.c_str(), value_ptr(*val));
}