#pragma once
//#include "pch.h"
#include "Game.h"
#include <sdl/SDL_main.h>

int main(int argc, char *argv[]) {
    Game game;
    game.init();
    return game.run();
}