#pragma once
#include "pch.h"
#include "Game.h"

#pragma warning(push, 0)

int main(int argc, char* argv[]) {
    Game game;
    game.init();
    while (game.is_running) {
        game.run();
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}

#pragma warning(pop)