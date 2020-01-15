#pragma once
#include "pch.h"
#include "Components.h"

inline void pollInputs(MouseKeyboardInput& mkb,
                       std::array<GamepadInput, 4>& pads) {
    SDL_PumpEvents();

    // Update mouse
    Uint32 newMouseButton = SDL_GetMouseState(&mkb.mousePos.x, &mkb.mousePos.y);
    for (size_t i = 0; i < mkb.numMouseButtons; ++i) {
        if (newMouseButton & SDL_BUTTON(i)) {
            if (!(mkb.mouseButtonMap & SDL_BUTTON(i))) {
                mkb.mouseButtonDownMap |= SDL_BUTTON(i);
            } else {
                mkb.mouseButtonDownMap &= ~SDL_BUTTON(i);
            }
            mkb.mouseButtonMap |= SDL_BUTTON(i);
            mkb.mouseButtonUpMap &= ~SDL_BUTTON(i);
        } else if (mkb.mouseButtonMap & SDL_BUTTON(i)) {
            mkb.mouseButtonMap &= ~SDL_BUTTON(i);
            mkb.mouseButtonDownMap &= ~SDL_BUTTON(i);
            mkb.mouseButtonUpMap |= SDL_BUTTON(i);
        } else {
            mkb.mouseButtonUpMap &= ~SDL_BUTTON(i);
        }
    }

    // Update keyboard keys
    for (int i = 0; i < mkb.numKeys; ++i) {
        if (mkb.sdlKeyboard[i]) {
            if (!mkb.key[i]) {
                mkb.keyDown[i] = true;
            } else {
                mkb.keyDown[i] = false;
            }
            mkb.key[i] = true;
        } else if (mkb.key[i]) {
            mkb.key[i] = false;
            mkb.keyDown[i] = false;
            mkb.keyUp[i] = true;
        } else {
            mkb.keyUp[i] = false;
        }
    }

    // Update gamepads
    for (size_t padIndex = 0;
         padIndex < static_cast<size_t>(GamepadInput::numGamepads);
         ++padIndex) {
        GamepadInput& pad = pads[padIndex];

        // Check if gamepad is still valid
        SDL_assert(pad.sdlPtr);

        // Poll all the axes on this pad
        Sint16 tempAxis;
        for (size_t i = 0; i < GamepadInput::numAxes; ++i) {

            // If it's a trigger, normalize and set the value
            if (i == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
                i == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                pad.axis[i] =
                    static_cast<float>(SDL_GameControllerGetAxis(
                        pad.sdlPtr, static_cast<SDL_GameControllerAxis>(i))) /
                    32767.0F;
                continue;
            }

            // If it's a stick, calculate and set it's value
            tempAxis = SDL_GameControllerGetAxis(
                pad.sdlPtr, static_cast<SDL_GameControllerAxis>(i));
            if (tempAxis > -GamepadInput::joyDeadzoneIn &&
                tempAxis < GamepadInput::joyDeadzoneIn) {
                pad.axis[i] = 0.0F;
            } else if (tempAxis > GamepadInput::joyDeadzoneOut) {
                pad.axis[i] = 1.0F;
            } else if (tempAxis < -GamepadInput::joyDeadzoneOut) {
                pad.axis[i] = -1.0F;
            } else {
                if (tempAxis > 0) {
                    pad.axis[i] =
                        static_cast<float>(tempAxis -
                                           GamepadInput::joyDeadzoneIn) /
                        static_cast<float>(GamepadInput::joyDeadzoneOut -
                                           GamepadInput::joyDeadzoneIn);
                } else {
                    pad.axis[i] =
                        static_cast<float>(tempAxis +
                                           GamepadInput::joyDeadzoneIn) /
                        static_cast<float>(GamepadInput::joyDeadzoneOut -
                                           GamepadInput::joyDeadzoneIn);
                }
            }
            // Invert Y axis cause it's the wrong way by default
            if (i == SDL_CONTROLLER_AXIS_LEFTY ||
                i == SDL_CONTROLLER_AXIS_RIGHTY)
                pad.axis[i] *= -1.0f;
        }

        // Poll all the buttons on this pad
        for (U32 i = 0; i < GamepadInput::numButtons; ++i) {
            if (SDL_GameControllerGetButton(
                    pad.sdlPtr, static_cast<SDL_GameControllerButton>(i))) {
                if (!pad.button(i)) {
                    setNthBitTo(pad.buttonDownMap, i, 1);
                } else {
                    setNthBitTo(pad.buttonDownMap, i, 0);
                }
                setNthBitTo(pad.buttonMap, i, 1);
            } else if (pad.button(i)) {
                setNthBitTo(pad.buttonDownMap, i, 0);
                setNthBitTo(pad.buttonUpMap, i, 1);
                setNthBitTo(pad.buttonMap, i, 0);
            } else {
                setNthBitTo(pad.buttonUpMap, i, 0);
            }
        }
    }
}

inline void updatePlayer(Entity& player,
                         ComponentContainer<BoxCollider>& colliders) {
    auto& pContr = *player.playerController;
    auto& pPos = player.transform->pos;
    auto& pVelo = pContr.velocity;
    auto& pHe = pContr.halfExt;

    // Gravity
    constexpr float gravity = 0.1f;
    pVelo.y += gravity;

    pVelo.x =
        pContr.walkSpeed * player.gamepadInput->axis[SDL_CONTROLLER_AXIS_LEFTX];

    // Collision check
    BoxCollider* groundCandidate = nullptr;

    // Find closest ground object
    for (auto& c : colliders) {
        auto topRight = c.topRight();
        auto botLeft = c.botLeft();
        if (((topRight.y < pPos.y + pHe.y) && (botLeft.y > pPos.y - pHe.y)) &&
            (topRight.x < pPos.x && botLeft.x > pPos.x)) {
            // c is inside the player from below
            if (!groundCandidate || groundCandidate->top() > topRight.y) {
                groundCandidate = &c;
            }
        }
    }

    if (groundCandidate) {
        pPos.y -= groundCandidate->top() - pPos.y + pHe.y;
        pContr.grounded = true;
    } else {
        pContr.grounded = false;
    }

    // Arm control
    constexpr float sensitivity = 0.4f;
    auto& bones = player.riggedMesh->bones;
    uint boneIndex = bones.getIndex("B_Arm_L");
    bones.rotation[boneIndex] = glm::rotate(
        bones.rotation[boneIndex],
        degToRad(-sensitivity *
                 player.gamepadInput->axis[SDL_CONTROLLER_AXIS_RIGHTY]),
        glm::vec3(0.0f, 0.0f, 1.0f));
}

inline void render(SDL_Window* window, RenderData renderData,
                   const std::vector<Entity>& entities) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    using namespace glm;

    const auto& rs = renderData.riggedShader;
    glUseProgram(rs.id);
    mat4 projection = ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(rs.projectionMatrixLoc, 1, GL_FALSE,
                       value_ptr(projection));

    for (auto& e : entities) {
        if (e.riggedMesh == nullptr || e.spriteRenderer == nullptr)
            continue;

        mat4 model(1.0f);
        // Entity transformations first to multiply from the left to
        // spriteRenderer transfotmation
        model = scale(model, vec3(e.transform->scale, 1.0f));
        model = rotate(model, e.transform->rot, vec3(0.0f, 0.0f, 1.0f));
        model = translate(model, vec3(e.transform->pos, 0.0f));

        // Move the center of the texture to the spriteRenderers position
        model = scale(model, vec3(e.spriteRenderer->tex.dimensions, 1.0f));
        model = translate(model, vec3(e.spriteRenderer->pos, 0.0f));
        glUniformMatrix4fv(rs.modelMatrixLoc, 1, GL_FALSE, value_ptr(model));
#ifndef SHADER_DEBUG
        RiggedMesh& rm = const_cast<RiggedMesh&>(*e.riggedMesh);
        mat4 boneTransforms[rm.maxBones];
        for (size_t i = 0; i < rm.maxBones; ++i) {
            mat4& boneInverse = rm.bones.inverseTransform[i];
            boneTransforms[i] =
                (inverse(boneInverse) * rm.bones.rotation[i] * boneInverse);
        }
        glUniformMatrix4fv(rs.bonesLoc, rm.maxBones, GL_FALSE,
                           value_ptr(boneTransforms[0]));
#else
        static_cast<RenderData>(renderData);

        RiggedMesh& rm = const_cast<RiggedMesh&>(e.riggedMesh);
        for (size_t i = 0; i < rm.vertices.size(); ++i) {
            Vertex v = rm.vertices[i];
            rm.debugVertices[i].uvCoord = v.uvCoord;

            mat4* boneInverse[2];
            boneInverse[0] = &rm.bones.inverseTransform[v.boneIndex[0]];
            boneInverse[1] = &rm.bones.inverseTransform[v.boneIndex[1]];
            mat4 boneTransform =
                (inverse(*boneInverse[0]) * rm.bones.rotation[v.boneIndex[0]] *
                 *boneInverse[0]) *
                v.boneWeight[0];
            boneTransform +=
                (inverse(*boneInverse[1]) * rm.bones.rotation[v.boneIndex[1]] *
                 *boneInverse[1]) *
                v.boneWeight[1];

            rm.debugVertices[i].pos =
                projection * model * boneTransform * vec4(v.position, 1.0f);
        }

        glNamedBufferSubData(
            rm.vbo, 0, sizeof(RiggedMesh::DebugVertex) * rm.vertices.size(),
            rm.debugVertices);
#endif
        glBindVertexArray(e.riggedMesh->vao);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, e.spriteRenderer->tex.id);

        glDrawElements(GL_TRIANGLES, e.riggedMesh->numIndices, GL_UNSIGNED_INT,
                       0);
    }

    // Unbind vao for error safety
    glBindVertexArray(0);

    SDL_GL_SwapWindow(window);
};
