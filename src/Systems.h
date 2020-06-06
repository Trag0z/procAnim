#pragma once
#include "pch.h"
#include "Mesh.h"

inline void update_gui(SDL_Window* window, RenderData& render_data,
                       GameConfig& game_config, const Player& player,
                       SplineEditor& spline_editor) {
    using namespace ImGui;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    NewFrame();

    //////          Debug controls window           //////
    Begin("Debug control", NULL, ImGuiWindowFlags_NoTitleBar);
    Checkbox("Render player.model", &render_data.draw_models);
    Checkbox("Render wireframes", &render_data.draw_wireframes);
    Checkbox("Render bones", &render_data.draw_bones);

    NewLine();
    SetNextItemWidth(100);
    DragFloat("Game speed", &game_config.speed, 0.1f, 0.0f, 100.0f, "%.2f");

    NewLine();
    Checkbox("Step mode", &game_config.step_mode);
    Checkbox("Spline edit mode", &game_config.spline_edit_mode);

    End();

    //////          Limb data display window            //////
    Begin("Limb data", NULL, ImGuiWindowFlags_NoTitleBar);

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

    for (const auto& anim : player.rigged_mesh.arm_animators) {
        glm::vec4 target_world_pos = player.model * anim.target_pos;
        sprintf_s(label, "%6.1f, %6.1f", target_world_pos.x,
                  target_world_pos.y);
        Text(label);
        NextColumn();
    }
    for (const auto& anim : player.rigged_mesh.leg_animators) {
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

    //////          Spline editor windows            //////
    if (game_config.spline_edit_mode)
        spline_editor.update_gui();

    End();
}

inline void render(SDL_Window* window, const RenderData& render_data,
                   Player& player, BoxCollider& ground,
                   SplineEditor& spline_editor) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    using namespace glm;

    // Render ground
    ground.model = translate(mat4(1.0f), vec3(ground.pos, 0.0f));

    for (size_t i = 0; i < ground.vertices.size(); ++i) {
        ground.shader_vertices[i].pos = ground.model * ground.vertices[i];
    }
    ground.vao.update_vertex_data(ground.shader_vertices);

    glUseProgram(render_data.debug_shader.id);
    glUniform4f(render_data.debug_shader.color_loc, 1.0f, 0.5f, 0.2f,
                1.0f); // ugly orange-ish
    ground.vao.draw(GL_TRIANGLES);

    // Translate first to multiply translation from the left to the scaled
    // player.model
    // This resolves to model * translation * scale
    player.model = player.model = translate(mat4(1.0f), vec3(player.pos, 0.0f));
    player.model = scale(player.model, player.scale);

    RiggedMesh& rm = player.rigged_mesh;

    // Calculate bone transforms from their rotations
    size_t bone_count = rm.bones.size();
    mat4* bone_transforms = new mat4[bone_count];
    for (size_t i = 0; i < bone_count; ++i) {
        auto& b = rm.bones[i];
        mat4 rotation_matrix =
            rotate(mat4(1.0f), b.rotation, vec3(0.0f, 0.0f, 1.0f));

        bone_transforms[i] = b.get_transform();
    }

    // Calculate vertex posistions for rendering
    for (size_t i = 0; i < rm.vertices.size(); ++i) {
        Vertex vert = rm.vertices[i];
        rm.shader_vertices[i].uv_coord = vert.uv_coord;

        mat4 bone = bone_transforms[vert.bone_index[0]] * vert.bone_weight[0] +
                    bone_transforms[vert.bone_index[1]] * vert.bone_weight[1];

        rm.shader_vertices[i].pos =
            player.model * bone * vec4(vert.position, 1.0f);
    }

    rm.vao.update_vertex_data(rm.shader_vertices);

    // Render player model
    if (render_data.draw_models) {
        glUseProgram(render_data.rigged_shader.id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, player.tex.id);

        rm.vao.draw(GL_TRIANGLES);
    }

    // Render wireframes
    if (render_data.draw_wireframes) {
        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 0.0f, 0.0f, 1.0f,
                    1.0f); // Red

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        rm.vao.draw(GL_TRIANGLES);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Render bones
    if (render_data.draw_bones) {
        for (size_t i = 0; i < rm.bones.size(); ++i) {
            rm.bones_shader_vertices[i * 2].pos =
                player.model * bone_transforms[i] *
                rm.bones[i]
                    .bind_pose_transform[3]; // Renders (0.0f, 0.0f, 0.0f) in
                                             // the bones local space
            rm.bones_shader_vertices[i * 2 + 1].pos =
                player.model * bone_transforms[i] *
                rm.bones[i].bind_pose_transform * rm.bones[i].tail;
        }

        rm.bones_vao.update_vertex_data(rm.bones_shader_vertices);

        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 1.0f, 0.0f, 0.0f,
                    1.0f); // Blue

        glLineWidth(2.0f);
        rm.bones_vao.draw(GL_LINES);

        rm.bones_vao.draw(GL_POINTS);
    }

    // Render animator target positions
    for (auto& anim : rm.arm_animators) {
        if (anim.target_pos.w == 0.0f)
            continue;

        vec4 render_pos = player.model * anim.target_pos;

        anim.vao.update_vertex_data(
            reinterpret_cast<DebugShaderVertex*>(&render_pos),
            1); // ugly, but it works

        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 0.0f, 1.0f, 0.0f, 1.0f);

        anim.vao.draw(GL_POINTS);
    }

    for (auto& anim : rm.leg_animators) {
        if (anim.target_pos.w == 0.0f)
            continue;

        vec4 render_pos = player.model * anim.target_pos;

        anim.vao.update_vertex_data(
            reinterpret_cast<DebugShaderVertex*>(&render_pos),
            1); // ugly, but it works

        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 0.0f, 1.0f, 0.0f, 1.0f);

        anim.vao.draw(GL_POINTS);
    }

    // Render splines
    spline_editor.render(render_data.debug_shader.id,
                         render_data.debug_shader.color_loc);

    // Unbind vao for error safety
    glBindVertexArray(0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
};
