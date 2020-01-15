#pragma once
#include "pch.h"
#include "Components.h"
#include "Entity.h"

template <typename T> struct ComponentContainer {
  private:
    static const size_t MAX_ELEMENTS = 10;

  public:
    size_t numElements = 0;
    std::array<T, MAX_ELEMENTS> elements;

    T* add(T&& e) {
        if (numElements == MAX_ELEMENTS) {
            SDL_assert(false);
            return nullptr;
        }
        elements[numElements] = e;
        return &elements[numElements++];
    }

	T* add(T& e) {
		if (numElements == MAX_ELEMENTS) {
			SDL_assert(false);
			return nullptr;
		}
		elements[numElements] = e;
		return &elements[numElements++];
	}

    // NOTE: Meybe these should forward or something?
    auto begin() noexcept { return elements.begin(); }
    auto end() noexcept { return elements.begin() + numElements; }
};

struct Game {
    bool running = false;
    U32 frameStart;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_GLContext glContext;

    std::vector<Entity> entities;

    // Singleton Components
    GameConfig gameConfig;
    RenderData renderData;
    MouseKeyboardInput mouseKeyboardInput;
    PlayerController playerController;

    // Component groups
    std::array<GamepadInput, 4> gamepadInputs;
    ComponentContainer<Transform> transforms;
    ComponentContainer<RiggedMesh> riggedMeshes;
    ComponentContainer<SpriteRenderer> spriteRenderers;
    ComponentContainer<BoxCollider> colliders;

    void init();
    bool run();
};