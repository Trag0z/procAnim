#pragma once
#include "pch.h"
#include "Game.h"

#pragma warning(push, 0)

int main(int argc, char *argv[]) {
    Game game;
    game.init();
    return game.run();
}

#pragma warning(pop)