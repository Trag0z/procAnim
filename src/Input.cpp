#pragma once
#include "Input.h"
#include "rendering/Renderer.h"

s32 Gamepad::STICK_DEADZONE_IN = 3000;
s32 Gamepad::STICK_DEADZONE_OUT = 32767 - 1000;

void MouseKeyboardInput::init(const Renderer* renderer_) {
    sdl_keyboard = SDL_GetKeyboardState(&num_keys);
    key_ = (bool*)malloc(sizeof(bool) * (num_keys * 3));
    key_down_ = key_ + num_keys;
    key_up_ = key_down_ + num_keys;

    renderer = renderer_;
}

void MouseKeyboardInput::update() {
    // Update mouse
    last_mouse_pos = mouse_pos;
    Uint32 new_mouse_button = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    for (size_t i = 0; i <= NUM_MOUSE_BUTTONS; ++i) {
        if (new_mouse_button & SDL_BUTTON(i)) {
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

    mouse_wheel_scroll = 0;
}

bool MouseKeyboardInput::mouse_button(MouseButton button) const {
    return mouse_button_map & static_cast<uint>(button);
}
bool MouseKeyboardInput::mouse_button_up(MouseButton button) const {
    return mouse_button_up_map & static_cast<uint>(button);
}
bool MouseKeyboardInput::mouse_button_down(MouseButton button) const {
    return mouse_button_down_map & static_cast<uint>(button);
}

bool MouseKeyboardInput::key(SDL_Scancode key) const { return key_[key]; }
bool MouseKeyboardInput::key_up(SDL_Scancode key) const { return key_up_[key]; }
bool MouseKeyboardInput::key_down(SDL_Scancode key) const {
    return key_down_[key];
}

glm::vec2 MouseKeyboardInput::mouse_pos_world() const noexcept {
    return renderer->screen_to_world_space(static_cast<glm::vec2>(mouse_pos));
}

glm::ivec2 MouseKeyboardInput::mouse_pos_screen() const noexcept {
    return mouse_pos;
}

glm::vec2 MouseKeyboardInput::mouse_move_world() const noexcept {
    return renderer->screen_to_world_space(mouse_pos) -
           renderer->screen_to_world_space(last_mouse_pos);
}

glm::ivec2 MouseKeyboardInput::mouse_move_screen() const noexcept {
    return mouse_pos - last_mouse_pos;
}

void Gamepad::init(size_t index) {
    sdl_ptr = SDL_GameControllerOpen(static_cast<int>(index));
    if (!sdl_ptr) {
        printf("[Input] Error opening gamepad %zd: %s. Using dummy pad.\n",
               index, SDL_GetError());

        for (auto& a : axes) {
            a = 0.0f;
        }
        button_map = button_down_map = button_up_map = 0;
    }
}

void Gamepad::update() {
    // Check if gamepad is still valid
    if (!sdl_ptr) {
        return;
    }

    // Poll all the axes on this pad
    Sint16 tempAxis;
    for (size_t i = 0; i < Gamepad::NUM_AXES; ++i) {

        // If it's a trigger, normalize and set the value
        if (i == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
            i == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
            axes[i] = static_cast<float>(SDL_GameControllerGetAxis(
                          sdl_ptr, static_cast<SDL_GameControllerAxis>(i))) /
                      32767.0F;
            continue;
        }

        // If it's a stick, calculate and set it's value
        const s32 effective_stick_deadzone_out =
            32767 - Gamepad::STICK_DEADZONE_OUT;

        tempAxis = SDL_GameControllerGetAxis(
            sdl_ptr, static_cast<SDL_GameControllerAxis>(i));
        if (tempAxis > -Gamepad::STICK_DEADZONE_IN &&
            tempAxis < Gamepad::STICK_DEADZONE_IN) {
            axes[i] = 0.0F;
        } else if (tempAxis > effective_stick_deadzone_out) {
            axes[i] = 1.0F;
        } else if (tempAxis < -effective_stick_deadzone_out) {
            axes[i] = -1.0F;
        } else {
            if (tempAxis > 0) {
                axes[i] =
                    static_cast<float>(tempAxis - Gamepad::STICK_DEADZONE_IN) /
                    static_cast<float>(effective_stick_deadzone_out -
                                       Gamepad::STICK_DEADZONE_IN);
            } else {
                axes[i] =
                    static_cast<float>(tempAxis + Gamepad::STICK_DEADZONE_IN) /
                    static_cast<float>(effective_stick_deadzone_out -
                                       Gamepad::STICK_DEADZONE_IN);
            }
        }
        // Invert Y axis cause it's the wrong way by default
        if (i == SDL_CONTROLLER_AXIS_LEFTY || i == SDL_CONTROLLER_AXIS_RIGHTY)
            axes[i] *= -1.0f;
    }

    // Poll all the buttons on this pad
    for (u32 i = 0; i < Gamepad::NUM_BUTTONS; ++i) {
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

glm::vec2 Gamepad::stick(StickID id) const {
    return glm::vec2(axes[static_cast<size_t>(id) * 2],
                     axes[static_cast<size_t>(id) * 2 + 1]);
}

bool Gamepad::button(u32 n) const { return button_map & BIT(n); }
bool Gamepad::button_down(u32 n) const { return button_down_map & BIT(n); }
bool Gamepad::button_up(u32 n) const { return button_up_map & BIT(n); }