#pragma once
#include "pch.h"
#include "Mesh.h"

inline void update_gui(SDL_Window* window, RenderData& render_data,
                       GameConfig& game_config, Player& player) {
    using namespace ImGui;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    NewFrame();

    //////          Debug controls window           //////
    Begin("Debug control", NULL, ImGuiWindowFlags_NoTitleBar);
    Checkbox("Render player model", &render_data.draw_models);
    Checkbox("Render wireframes", &render_data.draw_wireframes);
    Checkbox("Render bones", &render_data.draw_bones);
    Checkbox("Render splines", &render_data.draw_splines);
    Checkbox("Render leg circles", &render_data.draw_circles);

    NewLine();
    Checkbox("Use constant delta time", &game_config.use_const_delta_time);
    SetNextItemWidth(100);
    DragFloat("Game speed", &game_config.speed, 0.1f, 0.0f, 100.0f, "%.2f");

    NewLine();
    Checkbox("Step mode", &game_config.step_mode);
    Checkbox("Spline edit mode", &player.spline_edit_mode);

    End();

    //////          Limb data display window            //////
    Begin("Limb data", NULL);

    char label[64];

    sprintf_s(label, "% 6.1f, % 6.1f", player.pos.x, player.pos.y);
    Text("Player position: ");
    SameLine();
    DragFloat2("Player position", (float*)&player.pos, 1.0f, 0.0f, 0.0f,
               "% .2f");

    Text("Target Positions");
    Columns(4);
    Separator();
    Text("Arm_L");
    NextColumn();
    Text("Arm_R");
    NextColumn();
    Text("Leg_L");
    NextColumn();
    Text("Leg_R");
    NextColumn();
    Separator();

    for (const auto& anim : player.animator.arm_animators) {
        glm::vec4 target_world_pos = player.model * anim.target_pos;
        sprintf_s(label, "%6.1f, %6.1f", target_world_pos.x,
                  target_world_pos.y);
        Text(label);
        NextColumn();
    }
    for (const auto& anim : player.animator.leg_animators) {
        glm::vec4 target_world_pos = player.model * anim.target_pos;
        sprintf_s(label, "%6.1f, %6.1f", target_world_pos.x,
                  target_world_pos.y);
        Text(label);
        NextColumn();
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

        glm::vec4 tail_world_pos = player.model * bone.get_transform() *
                                   bone.bind_pose_transform * bone.tail;
        sprintf_s(label, "% 7.1f, % 7.1f", tail_world_pos.x, tail_world_pos.y);
        Text(label);
        NextColumn();
    }
    Columns(1);
    Separator();

    End();
}

inline void render(SDL_Window* window, const RenderData& render_data,
                   Player& player, BoxCollider& ground) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ground.render(render_data);

    player.render(render_data);

    // Unbind vao for error safety
    glBindVertexArray(0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
};
