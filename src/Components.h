#pragma once
//#include "pch.h"
#include "Shader.h"
#include "Texture.h"
#include "Types.h"
#include "Util.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <sdl/SDL.h>

using namespace glm;

struct Transform {
    vec2 pos;
    vec2 scale;
    GLfloat rot;
};

struct SpriteRenderer {
    GLuint vao;
    Texture *tex;
    Shader *shader;
};

struct FlatRenderer {
    GLuint vao;
    vec4 color;

    static GLuint shaderID;
    static GLint colorLocation;
    static void setColor(vec4 color) {
        glUniform4f(colorLocation, color.r, color.g, color.b, color.a);
    };
};

struct GamepadInput {
    static const U32 numAxes = SDL_CONTROLLER_AXIS_MAX;
    static const U32 numButtons = SDL_CONTROLLER_BUTTON_MAX;
    static const S32 joyDeadzoneIn = 8000;
    static const S32 joyDeadzoneOut = 32767 - 1000;

    static U32 numGamepads;

    SDL_GameController *sdlPtr = nullptr;

    float axis[numAxes];
    Uint32 buttonMap, buttonDownMap, buttonUpMap;

    // NOTE: These could only be shifted by n-1 if SDL_GAMECONTROLLER_BUTTON
    // starts at 1
    inline bool button(U32 n) const { return buttonMap & BIT(n); };
    inline bool buttonDown(U32 n) const { return buttonDownMap & BIT(n); };
    inline bool buttonUp(U32 n) const { return buttonUpMap & BIT(n); };
};

//                                       //
//           Singleton Components        //
//                                       //

struct MouseKeyboardInput {
    const static uint numMouseButtons = 3;
    const Uint8 *sdlKeyboard;
    int numKeys;

    bool *key, *keyUp, *keyDown;
    uint mouseButtonMap, mouseButtonDownMap, mouseButtonUpMap;
    ivec2 mousePos;

    void init() {
        sdlKeyboard = SDL_GetKeyboardState(&numKeys);
        key = (bool *)malloc(sizeof(bool) * (numKeys * 3));
        keyDown = key + numKeys;
        keyUp = keyDown + numKeys;
    };
};

struct GameConfig {
    const U32 fps = 60;
    const U32 frameDelay = 1000 / fps;
    const U32 windowFlags =
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;

    const GLclampf bgColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    Ivec2 windowSize = {1920, 1080};
};