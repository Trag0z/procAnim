#pragma once

#include "Audio.h"

void AudioManager::load_sounds() {
    chunks[Sound::HIT_1] = Mix_LoadWAV("../assets/sounds/hit1.wav");
    chunks[Sound::HIT_2] = Mix_LoadWAV("../assets/sounds/hit2.wav");
    chunks[Sound::WALL_BOUNCE] = Mix_LoadWAV("../assets/sounds/koopa.wav");

#ifdef _DEBUG
    for (auto& c : chunks) {
        SDL_assert(c.second != NULL);
    }
#endif
}

void AudioManager::play(Sound sound) {
    int ret = Mix_PlayChannel(-1, chunks[sound], 0);
    SDL_assert(ret != -1);
}