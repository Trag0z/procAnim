#pragma once
#include "Game.h"

// #pragma warning(push, 0)

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    Game game;
    game.init();
    while (game.is_running) {
        game.run();
    }

    IMG_Quit();
    SDL_Quit();
    return 0;
}

// #pragma warning(pop)