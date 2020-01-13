#pragma once
#include "pch.h"
#include "Components.h"

struct Entity {
    Transform transform;
    RiggedMesh riggedMesh;
    SpriteRenderer spriteRenderer;
    PlayerController playerController;
    GamepadInput* gamepadInput;
};