#pragma once
#include "pch.h"
#include "Game.h"
#include "DebugCallback.h"
#include "Systems.h"


constexpr bool DEBUG_MODE = true;

void Game::init() {
    // Initialize SDL
    SDL_assert_always(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    SDL_assert_always(IMG_Init(IMG_INIT_PNG) != 0);
    SDL_assert_always(TTF_Init() == 0);
    printf("SDL initialized\n");

    window = SDL_CreateWindow("procAnim", SDL_WINDOWPOS_CENTERED, 0,
                              gameConfig.windowSize.x, gameConfig.windowSize.y,
                              gameConfig.windowFlags);
    SDL_assert_always(window);
    printf("Window created\n");

    renderer = SDL_CreateRenderer(window, -1, 0);

    const int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; ++i) {
        if (!SDL_IsGameController(i)) {
            printf("[Input] Joystick%d is not a supported GameController!\n",
                   i);
            continue;
        }
        ++GamepadInput::numGamepads;
    }

    // Use OpenGL 3.1 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    if (DEBUG_MODE)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // Create context
    glContext = SDL_GL_CreateContext(window);
    if (glContext == NULL) {
        printf("OpenGL context could not be created! SDL Error: %s\n",
               SDL_GetError());
        SDL_assert(false);
    }
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
    }

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    // OpenGL configuration
    glViewport(0, 0, gameConfig.windowSize.x, gameConfig.windowSize.y);
    // glEnable(GL_CULL_FACE);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (DEBUG_MODE) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(handleGLDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                              nullptr, GL_TRUE);
    }

    // Initialize components
	Mesh::init();

    SpriteRenderer::init(loadAndCompileShaderFromFile(
        "../src/shaders/default.vert", "../src/shaders/default.frag"));

    FlatRenderer::init(loadAndCompileShaderFromFile(
        "../src/shaders/flat.vert", "../src/shaders/flat.frag"));


    mouseKeyboardInput.init();

    // Open gamepads
    for (U32 i = 0; i < GamepadInput::numGamepads; ++i) {
        gamepadInputs[i].sdlPtr = SDL_GameControllerOpen(i);
        if (!gamepadInputs[i].sdlPtr) {
            printf("[Input] Error opening gamepad%I32d: %s\n", i,
                   SDL_GetError());
        }
    }

    // Initialize entity
    Transform t;
    t.pos = {1920.0f / 2.0f, 1080.0f / 2.0f };
    t.rot = 0.0f;
    t.scale = {1.0f, 1.0f};

    SpriteRenderer s;
    s.pos = {0.0f, 0.0f};
    s.tex = loadTexture("../assets/testTex.png");

    entity.transform = t;
    entity.mesh = Mesh::simple();
    entity.spriteRenderer = s;

    running = true;
};

bool Game::run() {
    while (running) {
        frameStart = SDL_GetTicks();

        pollInputs(mouseKeyboardInput, gamepadInputs);

        render(window, entity);

        // Check for errors and clear error queue
        while (GLenum error = glGetError()) {
            printf("[OpenGL Error] %d\n", error);
            SDL_assert(!error);
        }

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