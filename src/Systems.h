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
};

inline void updatePlayer(Entity& entity) {
    constexpr float sensitivity = 0.1f;
    static_cast<Entity>(entity);
    // entity.riggedMesh.vertices[PlayerController::LEFT_HAND].position +=
    //     glm::vec3(
    //         entity.gamepadInput->axis[SDL_CONTROLLER_AXIS_RIGHTX] *
    //         sensitivity,
    //         entity.gamepadInput->axis[SDL_CONTROLLER_AXIS_RIGHTY] *
    //         sensitivity, 0.0f);

    // entity.riggedMesh.update();
}

inline void render(SDL_Window* window, RenderData renderData,
                   const Entity& entity) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    using namespace glm;

    mat4 projection = ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);

    mat4 model(1.0f);

    // Entity transformations first to multiply from the left to spriteRenderer
    // transfotmation
    model = scale(model, vec3(entity.transform.scale, 1.0f));
    model = rotate(model, entity.transform.rot, vec3(0.0f, 0.0f, 1.0f));
    model = translate(model, vec3(entity.transform.pos, 0.0f));

    // Move the center of the texture to the spriteRenderers position
    model = scale(model, vec3(entity.spriteRenderer.tex.dimensions, 1.0f));
    model = translate(model, vec3(entity.spriteRenderer.pos, 0.0f));

#ifndef SHADER_DEBUG
    glUseProgram(renderData.riggedShader.id);

    glUniformMatrix4fv(renderData.riggedShader.modelMatrix, 1, GL_FALSE,
                       value_ptr(model));
    glUniformMatrix4fv(renderData.riggedShader.projectionMatrix, 1, GL_FALSE,
                       value_ptr(projection));

    glUniformMatrix4fv(renderData.riggedShader.bones, 10, GL_TRUE,
                       value_ptr(entity.riggedMesh.boneOffsets[0]));

    glBindVertexArray(entity.riggedMesh.vao);
#else
    static_cast<RenderData>(renderData);

    RiggedMesh& rm = const_cast<RiggedMesh&>(entity.riggedMesh);
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
        boneTransform += (inverse(*boneInverse[1]) *
                          rm.bones.rotation[v.boneIndex[1]] * *boneInverse[1]) *
                         v.boneWeight[1];

        printGlm("Bone", boneTransform);

        // boneTransform += boneTransform2;
        // printGlm("BoneTransform", boneTransform);

        // printGlm("Model", model);
        // printGlm("Projection", projection);

        printGlm<glm::vec4>("Vertex before", vec4(v.position, 1.0f));
        rm.debugVertices[i].pos =
            projection * model * boneTransform * vec4(v.position, 1.0f);
        printGlm<glm::vec4>("Final position", rm.debugVertices[i].pos);
        printf("\n");
    }

    glNamedBufferSubData(rm.vbo, 0,
                         sizeof(RiggedMesh::DebugVertex) * rm.vertices.size(),
                         rm.debugVertices);

    glBindVertexArray(entity.riggedMesh.vao);
#endif

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, entity.spriteRenderer.tex.id);

    glDrawElements(GL_TRIANGLES, entity.riggedMesh.numIndices, GL_UNSIGNED_INT,
                   0);

    // unbind vao for error safety
    glBindVertexArray(0);

    SDL_GL_SwapWindow(window);
};
