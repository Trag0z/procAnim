#pragma once
#include "Game.h"
//#include "pch.h"
#include "ComponentUtil.h"
#include "Systems.h"
#include <gl/GLU.h>
#include <gl/glew.h>
#include <iostream>
#include <sdl/SDL_image.h>
#include <sdl/SDL_ttf.h>

void Game::init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("[SDL_Init] %s\n", SDL_GetError());
        SDL_assert(false);
    }
    if (!IMG_Init(IMG_INIT_PNG)) {
        printf("[IMG_Init] %s\n", SDL_GetError());
        SDL_assert(false);
    }
    if (TTF_Init() < 0) {
        printf("[TTF_Init] %s\n", SDL_GetError());
        SDL_assert(false);
    }
    printf("SDL initialized\n");

    window = SDL_CreateWindow("procAnim", SDL_WINDOWPOS_CENTERED, 0,
                              gameConfig.windowSize.x, gameConfig.windowSize.y,
                              gameConfig.windowFlags);
    if (!window) {
        printf("Failed to create window. SDL Error: %s\n", SDL_GetError());
        SDL_assert(false);
    } else {
        printf("Window created\n");
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; ++i) {
        if (!SDL_IsGameController(i)) {
            printf("[Input] Joystick%d is not a supported GameController!\n",
                   i);
            continue;
        }
        ++GamepadInput::numGamepads;
    }

    // Initialize Components
    mouseKeyboardInput.init();

    // Open gamepads
    for (U32 i = 0; i < GamepadInput::numGamepads; ++i) {
        gamepadInputs[i].sdlPtr = SDL_GameControllerOpen(i);
        if (!gamepadInputs[i].sdlPtr) {
            printf("[Input] Error opening gamepad%I32d: %s\n", i,
                   SDL_GetError());
        }
    }

    // Use OpenGL 3.1 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    // Create context
    gContext = SDL_GL_CreateContext(window);
    if (gContext == NULL) {
        printf("OpenGL context could not be created! SDL Error: %s\n",
               SDL_GetError());
    } else {
        // Initialize GLEW
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            printf("Error initializing GLEW! %s\n",
                   glewGetErrorString(glewError));
        }

        // Use Vsync
        if (SDL_GL_SetSwapInterval(1) < 0) {
            printf("Warning: Unable to set VSync! SDL Error: %s\n",
                   SDL_GetError());
        }

        loadAndCompileShaderFromFile(shader, "../src/default.vert",
                                     "../src/default.frag");

        Shader flatShader;
        loadAndCompileShaderFromFile(flatShader, "../src/flat.vert",
                                     "../src/flat.frag");

        FlatRenderer::shaderID = flatShader.id;
        FlatRenderer::colorLocation =
            glGetUniformLocation(flatShader.id, "outcolor");

        flatRenderer = createFlatRenderer();
        flatRenderer.color = {1.0f, 0.0f, 0.0f, 1.0f};
        FlatRenderer::setColor(flatRenderer.color);

        // OpenGL configuration
        glViewport(0, 0, gameConfig.windowSize.x, gameConfig.windowSize.y);
        // glEnable(GL_CULL_FACE);
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    running = true;
};

bool Game::run() {
    while (running) {
        frameStart = SDL_GetTicks();

        pollInputs(mouseKeyboardInput, gamepadInputs);

        render(window, transform, flatRenderer);

        // Wait for next frame
        U32 frameTime = SDL_GetTicks() - frameStart;
        if (gameConfig.frameDelay > frameTime)
            SDL_Delay(gameConfig.frameDelay - frameTime);
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
};