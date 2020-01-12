#pragma once
#include "pch.h"
#include "Components.h"

struct Entity {
    Transform transform;
    MutableMesh mesh;
    SpriteRenderer spriteRenderer;
    PlayerController playerController;
    GamepadInput* gamepadInput;
};