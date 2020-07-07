#pragma once
#include "pch.h"
#include "Input.h"
#include "Renderer.h"

void MouseKeyboardInput::update() {
    // Update mouse
    Uint32 newMouseButton = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    for (size_t i = 0; i < num_mouse_buttons; ++i) {
        if (newMouseButton & SDL_BUTTON(i)) {
            if (!(mouse_button_map & SDL_BUTTON(i))) {
                mouse_button_down_map |= SDL_BUTTON(i);
            } else {
                mouse_button_down_map &= ~SDL_BUTTON(i);
            }
            mouse_button_map |= SDL_BUTTON(i);
            mouse_button_up_map &= ~SDL_BUTTON(i);
        } else if (mouse_button_map & SDL_BUTTON(i)) {
            mouse_button_map &= ~SDL_BUTTON(i);
            mouse_button_down_map &= ~SDL_BUTTON(i);
            mouse_button_up_map |= SDL_BUTTON(i);
        } else {
            mouse_button_up_map &= ~SDL_BUTTON(i);
        }
    }

    // Update keyboard keys
    for (int i = 0; i < num_keys; ++i) {
        if (sdl_keyboard[i]) {
            if (!key_[i]) {
                key_down_[i] = true;
            } else {
                key_down_[i] = false;
            }
            key_[i] = true;
        } else if (key_[i]) {
            key_[i] = false;
            key_down_[i] = false;
            key_up_[i] = true;
        } else {
            key_up_[i] = false;
        }
    }
}

std::array<Gamepad, Gamepad::num_pads> Gamepad::init() {
    std::array<Gamepad, num_pads> ret;
    for (U32 i = 0; i < Gamepad::num_pads; ++i) {
        ret[i].sdl_ptr = SDL_GameControllerOpen(i);
        if (!ret[i].sdl_ptr) {
            printf("[Input] Error opening gamepad%I32d: %s\n", i,
                   SDL_GetError());
        }
    }
    return ret;
}

glm::vec2 MouseKeyboardInput::mouse_world_pos() const {
    return renderer->camera_position() + mouse_screen_pos();
}

glm::vec2 MouseKeyboardInput::mouse_screen_pos() const {
    return glm::vec2(static_cast<float>(mouse_pos.x),
                     renderer->window_size().y -
                         static_cast<float>(mouse_pos.y));
}

void Gamepad::update() {
    // Check if gamepad is still valid
    SDL_assert(sdl_ptr);

    // Poll all the axes on this pad
    Sint16 tempAxis;
    for (size_t i = 0; i < Gamepad::num_axes; ++i) {

        // If it's a trigger, normalize and set the value
        if (i == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
            i == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
            axes[i] = static_cast<float>(SDL_GameControllerGetAxis(
                          sdl_ptr, static_cast<SDL_GameControllerAxis>(i))) /
                      32767.0F;
            continue;
        }

        // If it's a stick, calculate and set it's value
        tempAxis = SDL_GameControllerGetAxis(
            sdl_ptr, static_cast<SDL_GameControllerAxis>(i));
        if (tempAxis > -Gamepad::stick_deadzone_in &&
            tempAxis < Gamepad::stick_deadzone_in) {
            axes[i] = 0.0F;
        } else if (tempAxis > Gamepad::stick_deadzone_out) {
            axes[i] = 1.0F;
        } else if (tempAxis < -Gamepad::stick_deadzone_out) {
            axes[i] = -1.0F;
        } else {
            if (tempAxis > 0) {
                axes[i] =
                    static_cast<float>(tempAxis - Gamepad::stick_deadzone_in) /
                    static_cast<float>(Gamepad::stick_deadzone_out -
                                       Gamepad::stick_deadzone_in);
            } else {
                axes[i] =
                    static_cast<float>(tempAxis + Gamepad::stick_deadzone_in) /
                    static_cast<float>(Gamepad::stick_deadzone_out -
                                       Gamepad::stick_deadzone_in);
            }
        }
        // Invert Y axis cause it's the wrong way by default
        if (i == SDL_CONTROLLER_AXIS_LEFTY || i == SDL_CONTROLLER_AXIS_RIGHTY)
            axes[i] *= -1.0f;
    }

    // Poll all the buttons on this pad
    for (U32 i = 0; i < Gamepad::num_buttons; ++i) {
        if (SDL_GameControllerGetButton(
                sdl_ptr, static_cast<SDL_GameControllerButton>(i))) {
            if (!button(i)) {
                setNthBitTo(button_down_map, i, 1);
            } else {
                setNthBitTo(button_down_map, i, 0);
            }
            setNthBitTo(button_map, i, 1);
        } else if (button(i)) {
            setNthBitTo(button_down_map, i, 0);
            setNthBitTo(button_up_map, i, 1);
            setNthBitTo(button_map, i, 0);
        } else {
            setNthBitTo(button_up_map, i, 0);
        }
    }
}
