#pragma once
#include "pch.h"

struct GameConfig;
class Renderer;

class ConfigLoader {
    GameConfig* game_config_;
    Renderer* renderer_;

    std::string save_path;

  public:
    void load_config(const char* path, GameConfig* game_config,
                     Renderer* renderer_);
    void save_config(const char* path = nullptr);
};
