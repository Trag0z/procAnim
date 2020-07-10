#pragma once
#include "pch.h"
#include "Types.h"
#include "Util.h"

class Renderer;

class MouseKeyboardInput {
    const static uint NUM_MOUSE_BUTTONS = 3;
    const Uint8* sdl_keyboard;
    int num_keys;

    bool *key_, *key_up_, *key_down_;
    uint mouse_button_map, mouse_button_down_map, mouse_button_up_map;
    glm::ivec2 mouse_pos, last_mouse_pos;

    const Renderer* renderer;

  public:
    int mouse_wheel_scroll;

    inline void init(const Renderer* renderer_) {
        sdl_keyboard = SDL_GetKeyboardState(&num_keys);
        key_ = (bool*)malloc(sizeof(bool) * (num_keys * 3));
        key_down_ = key_ + num_keys;
        key_up_ = key_down_ + num_keys;

        renderer = renderer_;
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

    glm::vec2 mouse_pos_world() const noexcept;
    glm::vec2 mouse_screen_pos() const noexcept;

    glm::vec2 mouse_move() const noexcept;
};

enum StickID { LEFT = 0, RIGHT = 1, TRIGGERS = 2 };

class Gamepad {
    static const U32 num_axes = SDL_CONTROLLER_AXIS_MAX;
    static const U32 num_buttons = SDL_CONTROLLER_BUTTON_MAX;
    static const S32 stick_deadzone_in = 8000;
    static const S32 stick_deadzone_out = 32767 - 1000;

    SDL_GameController* sdl_ptr = nullptr;

    float axes[num_axes];
    Uint32 button_map, button_down_map, button_up_map;

  public:
    static const U32 NUM_PADS = 1;

    static std::array<Gamepad, NUM_PADS> init();

    void update();

    inline glm::vec2 stick(size_t index) const {
        return glm::vec2(axes[index * 2], axes[index * 2 + 1]);
    }

    // NOTE: These could only be shifted by n-1 if SDL_GAMECONTROLLER_BUTTON
    // starts at 1
    inline bool button(U32 n) const { return button_map & BIT(n); };
    inline bool button_down(U32 n) const { return button_down_map & BIT(n); };
    inline bool button_up(U32 n) const { return button_up_map & BIT(n); };
};