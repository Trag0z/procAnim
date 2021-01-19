#pragma once
#include "pch.h"

enum class Sound { HIT_1, HIT_2, WALL_BOUNCE };

class AudioManager {
    std::map<Sound, Mix_Chunk*> chunks;

  public:
    void load_sounds();

    void play(Sound sound);
};