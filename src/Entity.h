#pragma once
#include "pch.h"
#include "Components.h"

struct Entity {
	Transform transform;
	Mesh mesh;
	SpriteRenderer spriteRenderer;
};