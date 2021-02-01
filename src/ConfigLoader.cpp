#pragma once
#include "pch.h"
#include "ConfigLoader.h"
#include "Types.h"
#include "Game.h"

static void parse_value(s32& dst, std::stringstream& src) {
    std::string word;

    std::getline(src, word);
    SDL_assert(word.length() >= 0);
    dst = static_cast<s32>(std::stoi(word));
}

static void parse_value(float& dst, std::stringstream& src) {
    std::string word;

    std::getline(src, word);
    SDL_assert(word.length() >= 0);
    dst = std::stof(word);
}

static void parse_value(glm::ivec2& dst, std::stringstream& src) {
    std::string word;

    std::getline(src, word, ',');
    SDL_assert(word.length() >= 0);
    dst.x = std::stoi(word);

    std::getline(src, word);
    SDL_assert(word.length() >= 0);
    dst.y = std::stoi(word.c_str());
}

static void parse_value(glm::vec2& dst, std::stringstream& src) {
    std::string word;

    std::getline(src, word, ',');
    SDL_assert(word.length() >= 0);
    dst.x = std::stof(word);

    std::getline(src, word);
    SDL_assert(word.length() >= 0);
    dst.y = std::stof(word.c_str());
}

void ConfigLoader::load_config(const char* path, GameConfig* game_config,
                               Renderer* renderer) {
    SDL_assert(save_path.empty());
    save_path = std::string(path);
    game_config_ = game_config;
    renderer_ = renderer;

    std::ifstream file_stream(path);
    SDL_assert(file_stream.is_open());

    for (std::string line; std::getline(file_stream, line);) {
        if (line.empty() || line.at(0) == '#') {
            continue;
        }

        std::stringstream stream{line};

        std::string word;
        std::getline(stream, word, ' ');

        if (word == "window_position") {
            parse_value(game_config_->window_position, stream);
        } else if (word == "gravity") {
            parse_value(game_config_->gravity, stream);
        } else if (word == "window_size") {
            parse_value(renderer_->window_size_, stream);
        } else if (word == "stick_deadzone_in") {
            parse_value(Gamepad::STICK_DEADZONE_IN, stream);
        } else if (word == "stick_deadzone_out") {
            parse_value(Gamepad::STICK_DEADZONE_OUT, stream);
            Gamepad::STICK_DEADZONE_OUT =
                Gamepad::MAX_STICK_VALUE - Gamepad::STICK_DEADZONE_OUT;
        }
    }
}