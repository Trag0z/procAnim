#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Game.h"
#include "Animator.h"

inline void update_gui(SDL_Window* window, Renderer& renderer,
                       GameConfig& game_config, Player& player) {
    using namespace ImGui;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    NewFrame();

    //////          Debug controls window           //////
    Begin("Debug control", NULL, ImGuiWindowFlags_NoTitleBar);
    Checkbox("Render player model", &renderer.draw_models);
    Checkbox("Render wireframes", &renderer.draw_wireframes);
    Checkbox("Render bones", &renderer.draw_bones);
    Checkbox("Render all splines", &renderer.draw_walk_splines);

    NewLine();
    Checkbox("Use constant delta time", &game_config.use_const_delta_time);
    SetNextItemWidth(100);
    DragFloat("Game speed", &game_config.speed, 0.1f, 0.0f, 100.0f, "%.2f");
    SetNextItemWidth(100);
    DragFloat(&player.animator.STEP_DISTANCE_MULTIPLIER);

    NewLine();
    Checkbox("Arm follows mouse", &player.animator.arm_follows_mouse);
    Checkbox("Step mode", &game_config.step_mode);
    Checkbox("Spline edit mode", &player.spline_edit_mode);
    Checkbox("Level editor", &game_config.level_edit_mode);

    End();

    //////          Limb data display window            //////
    Begin("Limb data", NULL);

    char label[128];

    sprintf_s(label, "% 6.1f, % 6.1f", player.position.x, player.position.y);
    Text("Player position: ");
    SameLine();
    bool changed_value =
        DragFloat2("Player position", value_ptr(player.position), 1.0f, 0.0f,
                   0.0f, "% .2f");
    if (changed_value) {
        player.grounded = false;
    }

    Columns(1);
    Separator();

    NewLine();
    Text("Limb data");
    Columns(3);
    Separator();
    Text("Name");
    NextColumn();
    Text("Rotation deg/rad");
    NextColumn();
    Text("Tail Position");
    NextColumn();
    Separator();

    for (const auto& bone : player.rigged_mesh.bones) {
        Text(bone.name.c_str());
        NextColumn();

        sprintf_s(label, "% 6.1f /% 1.2f", radToDeg(bone.rotation),
                  bone.rotation);
        Text(label);
        NextColumn();

        glm::vec3 tail_world_pos = player.model * bone.get_transform() *
                                   bone.bind_pose_transform *
                                   glm::vec3(bone.tail, 1.0f);
        sprintf_s(label, "% 7.1f, % 7.1f", tail_world_pos.x, tail_world_pos.y);
        Text(label);
        NextColumn();
    }
    Columns(1);
    Separator();

    End();

    if (player.spline_edit_mode)
        player.animator.spline_editor.update_gui(player.spline_edit_mode);
}
