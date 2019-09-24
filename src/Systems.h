#pragma once
#include "Components.h"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

using namespace glm;

inline void pollInputs(MouseKeyboardInput &mkb,
                       std::array<GamepadInput, 4> &pads) {
    SDL_PumpEvents();

    // Update mouse
    Uint32 newMouseButton = SDL_GetMouseState(&mkb.mousePos.x, &mkb.mousePos.y);
    for (size_t i = 0; i < mkb.numMouseButtons; ++i) {
        if (newMouseButton & SDL_BUTTON(i)) {
            if (!(mkb.mouseButtonMap & SDL_BUTTON(i))) {
                mkb.mouseButtonDownMap |= SDL_BUTTON(i);
            } else {
                mkb.mouseButtonDownMap &= ~SDL_BUTTON(i);
            }
            mkb.mouseButtonMap |= SDL_BUTTON(i);
            mkb.mouseButtonUpMap &= ~SDL_BUTTON(i);
        } else if (mkb.mouseButtonMap & SDL_BUTTON(i)) {
            mkb.mouseButtonMap &= ~SDL_BUTTON(i);
            mkb.mouseButtonDownMap &= ~SDL_BUTTON(i);
            mkb.mouseButtonUpMap |= SDL_BUTTON(i);
        } else {
            mkb.mouseButtonUpMap &= ~SDL_BUTTON(i);
        }
    }

    // Update keyboard keys
    for (int i = 0; i < mkb.numKeys; ++i) {
        if (mkb.sdlKeyboard[i]) {
            if (!mkb.key[i]) {
                mkb.keyDown[i] = true;
            } else {
                mkb.keyDown[i] = false;
            }
            mkb.key[i] = true;
        } else if (mkb.key[i]) {
            mkb.key[i] = false;
            mkb.keyDown[i] = false;
            mkb.keyUp[i] = true;
        } else {
            mkb.keyUp[i] = false;
        }
    }

    // Update gamepads
    for (U32 i = 0; i < GamepadInput::numGamepads; ++i) {
        GamepadInput pad = pads[i];

        // Check if gamepad is still valid
        SDL_assert(pad.sdlPtr);

        // Poll all the axes on this pad
        Sint16 tempAxis;
        for (size_t i = 0; i < GamepadInput::numAxes; ++i) {

            // If it's a trigger, normalize and set the value
            if (i == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
                i == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                pad.axis[i] =
                    static_cast<float>(SDL_GameControllerGetAxis(
                        pad.sdlPtr, static_cast<SDL_GameControllerAxis>(i))) /
                    32767.0F;
                continue;
            }

            // If it's a stick, calculate and set it's value
            tempAxis = SDL_GameControllerGetAxis(
                pad.sdlPtr, static_cast<SDL_GameControllerAxis>(i));
            if (tempAxis > -GamepadInput::joyDeadzoneIn &&
                tempAxis < GamepadInput::joyDeadzoneIn) {
                pad.axis[i] = 0.0F;
            } else if (tempAxis > GamepadInput::joyDeadzoneOut) {
                pad.axis[i] = 1.0F;
            } else if (tempAxis < -GamepadInput::joyDeadzoneOut) {
                pad.axis[i] = -1.0F;
            } else {
                if (tempAxis > 0) {
                    pad.axis[i] =
                        static_cast<float>(tempAxis -
                                           GamepadInput::joyDeadzoneIn) /
                        static_cast<float>(GamepadInput::joyDeadzoneOut -
                                           GamepadInput::joyDeadzoneIn);
                } else {
                    pad.axis[i] =
                        static_cast<float>(tempAxis +
                                           GamepadInput::joyDeadzoneIn) /
                        static_cast<float>(GamepadInput::joyDeadzoneOut -
                                           GamepadInput::joyDeadzoneIn);
                }
            }
            // Invert Y axis cause it's the wrong way by default
            if (i == SDL_CONTROLLER_AXIS_LEFTY ||
                i == SDL_CONTROLLER_AXIS_RIGHTY)
                pad.axis[i] *= -1.0f;
        }

        // Poll all the buttons on this pad
        for (U32 i = 0; i < GamepadInput::numButtons; ++i) {
            if (SDL_GameControllerGetButton(
                    pad.sdlPtr, static_cast<SDL_GameControllerButton>(i))) {
                if (!pad.button(i)) {
                    setNthBitTo(pad.buttonDownMap, i, 1);
                } else {
                    setNthBitTo(pad.buttonDownMap, i, 0);
                }
                setNthBitTo(pad.buttonMap, i, 1);
            } else if (pad.button(i)) {
                setNthBitTo(pad.buttonDownMap, i, 0);
                setNthBitTo(pad.buttonUpMap, i, 1);
                setNthBitTo(pad.buttonMap, i, 0);
            } else {
                setNthBitTo(pad.buttonUpMap, i, 0);
            }
        }
    }
};

inline void render(SDL_Window *window, Transform transform,
                   FlatRenderer flatRenderer) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(FlatRenderer::shaderID);
    FlatRenderer::setColor(flatRenderer.color);
    // TODO: enable correct vertex  array
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // mat4 projection = ortho(0.0f, 1920.0f, 1080.0f, 0.0f, -1.0f, 1.0f);

    // mat4 model;
    // model = translate(model, vec3(transform.pos, 0.0f));

    // // Translate so (0,0) is in the middle of the object, then rotate,
    // then
    // // translate back
    // model = translate(
    //     model, vec3(0.5f * transform.scale.x, 0.5f * transform.scale.y,
    //     0.0f));
    // model = rotate(model, transform.rot, vec3(0.0f, 0.0f, 1.0f));
    // model = translate(model, vec3(-0.5f * transform.scale.x,
    //                               -0.5f * transform.scale.y, 0.0f));

    // model = glm::scale(model, vec3(transform.scale, 1.0f));

    SDL_GL_SwapWindow(window);
};
