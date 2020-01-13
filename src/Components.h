#pragma once
#include "pch.h"
#include "components/Mesh.h"
#include "Shaders.h"
#include "Texture.h"
#include "Types.h"
#include "Util.h"

struct Transform {
    glm::vec2 pos;
    glm::vec2 scale;
    GLfloat rot;
};

struct PlayerController {
    enum MovePoint { LEFT_HAND = 6 };
};

struct SpriteRenderer {
    glm::vec2 pos;
    Texture tex;
};

struct GamepadInput {
    static const U32 numAxes = SDL_CONTROLLER_AXIS_MAX;
    static const U32 numButtons = SDL_CONTROLLER_BUTTON_MAX;
    static const S32 joyDeadzoneIn = 8000;
    static const S32 joyDeadzoneOut = 32767 - 1000;

    static U32 numGamepads;

    SDL_GameController* sdlPtr = nullptr;

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
    const Uint8* sdlKeyboard;
    int numKeys;

    bool *key, *keyUp, *keyDown;
    uint mouseButtonMap, mouseButtonDownMap, mouseButtonUpMap;
    glm::ivec2 mousePos;

    void init() {
        sdlKeyboard = SDL_GetKeyboardState(&numKeys);
        key = (bool*)malloc(sizeof(bool) * (numKeys * 3));
        keyDown = key + numKeys;
        keyUp = keyDown + numKeys;
    };
};

struct GameConfig {
    const U32 fps = 60;
    const U32 frameDelay = 1000 / fps;
    const U32 windowFlags =
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;

    glm::ivec2 windowSize = {1920, 1080};
};

struct RenderData {
    struct {
        GLuint id, modelMatrix, projectionMatrix;
    } simpleShader;

    struct {
        GLuint id, modelMatrix, projectionMatrix, bones;
    } riggedShader;

    void init(GLuint simpleShaderId, GLuint riggedShaderId);
};