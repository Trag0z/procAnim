#pragma once
#include "Components.h"
#include <glm/glm.hpp>

struct Entity {
	Transform transform;
	Mesh mesh;
	SpriteRenderer spriteRenderer;
};