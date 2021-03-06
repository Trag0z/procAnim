#pragma once
#include <sdl/SDL.h>
#include "Types.h"
#include "Util.h"
#include "ConfigManager.h"

class Renderer;

enum class MouseButton : uint { LEFT = 1, MIDDLE = 2, RIGHT = 4 };

class MouseKeyboardInput {
    const static uint NUM_MOUSE_BUTTONS = 3;
    const u8* sdl_keyboard;
    int num_keys;

    bool *key_, *key_up_, *key_down_;
    uint mouse_button_map, mouse_button_down_map, mouse_button_up_map;
    glm::ivec2 mouse_pos, last_mouse_pos;

    const Renderer* renderer;

  public:
    // Game::run() sets this if there was a scroll event. Would
    // be cleaner if this could be a private member that this class could update
    // itself, but I could not find a way to do it that way.
    int mouse_wheel_scroll;

    void init(const Renderer* renderer_);
    void update();

    bool mouse_button(MouseButton button) const;
    bool mouse_button_up(MouseButton button) const;
    bool mouse_button_down(MouseButton button) const;

    bool key(SDL_Scancode key) const;
    bool key_up(SDL_Scancode key) const;
    bool key_down(SDL_Scancode key) const;

    glm::vec2 mouse_pos_world() const noexcept;
    glm::ivec2 mouse_pos_screen() const noexcept;

    glm::vec2 mouse_move_world() const noexcept;
    glm::ivec2 mouse_move_screen() const noexcept;
};

enum class StickID : size_t { LEFT = 0, RIGHT = 1, TRIGGERS = 2 };

class Gamepad {
    static constexpr s16 MAX_STICK_VALUE = 32767;

    static const u32 NUM_AXES    = SDL_CONTROLLER_AXIS_MAX;
    static const u32 NUM_BUTTONS = SDL_CONTROLLER_BUTTON_MAX;
    static s16 STICK_DEADZONE_IN;
    static s16 STICK_DEADZONE_OUT;

    float axes[NUM_AXES];
    u32 button_map, button_down_map, button_up_map;

  public:
    SDL_GameController* sdl_ptr;

    Gamepad();
    void init(size_t index);
    void update();

    glm::vec2 stick(StickID id) const;

    bool button(u32 n) const;
    bool button_down(u32 n) const;
    bool button_up(u32 n) const;

    friend ConfigManager;
};