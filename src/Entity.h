#pragma once
#include "pch.h"
#include "Components.h"

struct Entity {
    Transform* transform = nullptr;
    RiggedMesh* riggedMesh = nullptr;
    SpriteRenderer* spriteRenderer = nullptr;
    PlayerController* playerController = nullptr;
    GamepadInput* gamepadInput = nullptr;
};