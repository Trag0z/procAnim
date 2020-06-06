#pragma once
#include "pch.h"
#include "Types.h"
#include "Util.h"

class MouseKeyboardInput {
    const static uint num_mouse_buttons = 3;
    const Uint8* sdl_keyboard;
    int num_keys;

    bool *key_, *key_up_, *key_down_;
    uint mouse_button_map, mouse_button_down_map, mouse_button_up_map;
    glm::ivec2 mouse_pos;

    int window_height;

  public:
    inline void init(int window_height_) {
        sdl_keyboard = SDL_GetKeyboardState(&num_keys);
        key_ = (bool*)malloc(sizeof(bool) * (num_keys * 3));
        key_down_ = key_ + num_keys;
        key_up_ = key_down_ + num_keys;

        window_height = window_height_;
    }

    void update();

    inline bool mouse_button(uint button) const {
        return mouse_button_map & button;
    }
    inline bool mouse_button_up(uint button) const {
        return mouse_button_up_map & button;
    }
    inline bool mouse_button_down(uint button) const {
        return mouse_button_down_map & button;
    }

    inline bool key(SDL_Scancode key) const { return key_[key]; }
    inline bool key_up(SDL_Scancode key) const { return key_up_[key]; }
    inline bool key_down(SDL_Scancode key) const { return key_down_[key]; }

    inline glm::vec2 mouse_world_pos() const {
        return glm::vec2(static_cast<float>(mouse_pos.x),
                         static_cast<float>(window_height - mouse_pos.y));
    }
};

class Gamepad {
    static const U32 num_axes = SDL_CONTROLLER_AXIS_MAX;
    static const U32 num_buttons = SDL_CONTROLLER_BUTTON_MAX;
    static const S32 stick_deadzone_in = 8000;
    static const S32 stick_deadzone_out = 32767 - 1000;

    SDL_GameController* sdl_ptr = nullptr;

    float axis[num_axes];
    Uint32 button_map, button_down_map, button_up_map;

  public:
    static const U32 num_pads = 0;

    static void init(std::array<Gamepad, num_pads> pads);

    void update();

    // NOTE: These could only be shifted by n-1 if SDL_GAMECONTROLLER_BUTTON
    // starts at 1
    inline bool button(U32 n) const { return button_map & BIT(n); };
    inline bool button_down(U32 n) const { return button_down_map & BIT(n); };
    inline bool button_up(U32 n) const { return button_up_map & BIT(n); };
};