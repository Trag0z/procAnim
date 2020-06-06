#pragma once
#include "pch.h"
#include "Spline.h"

Spline::Spline(glm::vec2 points_[num_points]) {
    memcpy_s(points, 4 * sizeof(glm::vec2), points_, 4 * sizeof(glm::vec2));

    // Init line render data
    glm::mat4 parameter_matrix = glm::mat4(
        glm::vec4(points[0], 0.0f, 1.0f), glm::vec4(points[3], 0.0f, 1.0f),
        glm::vec4(points[1], 0.0f, 1.0f), glm::vec4(points[2], 0.0f, 1.0f));

    GLuint indices[render_steps];
    glm::vec4 interpolation_vector;
    for (size_t i = 0; i < render_steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(render_steps);
        interpolation_vector = {t * t * t, t * t, t, 1.0f};
        line_shader_vertices[i].pos =
            parameter_matrix * hermite_matrix * interpolation_vector;

        indices[i] = static_cast<GLuint>(i);
    }
    line_vao.init(indices, render_steps, line_shader_vertices.data(),
                  static_cast<GLuint>(line_shader_vertices.size()));

    // Init point render data
    for (size_t i = 0; i < num_points; ++i) {
        point_shader_vertices[i].pos = glm::vec4(points[i], 0.0f, 1.0f);
    }
    point_vao.init(indices, num_points, point_shader_vertices.data(),
                   static_cast<GLuint>(point_shader_vertices.size()));
}

void Spline::update_render_data() {
    // Line
    glm::mat4 parameter_matrix = glm::mat4(
        glm::vec4(points[0], 0.0f, 1.0f), glm::vec4(points[3], 0.0f, 1.0f),
        glm::vec4(points[1], 0.0f, 1.0f), glm::vec4(points[2], 0.0f, 1.0f));

    glm::vec4 interpolation_vector;
    for (size_t i = 0; i < render_steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(render_steps - 1);
        interpolation_vector = {t * t * t, t * t, t, 1.0f};

        line_shader_vertices[i].pos =
            parameter_matrix * hermite_matrix * interpolation_vector;
    }

    line_vao.update_vertex_data(
        line_shader_vertices.data(),
        static_cast<GLuint>(line_shader_vertices.size()));

    // Points
    for (size_t i = 0; i < num_points; ++i) {
        point_shader_vertices[i].pos = glm::vec4(points[i], 0.0f, 1.0f);
    }
    point_vao.update_vertex_data(
        point_shader_vertices.data(),
        static_cast<GLuint>(point_shader_vertices.size()));
}

void SplineEditor::update(const MouseKeyboardInput& input) {
    if (creating_new_spline) {
        Spline& last = splines.back();
        if (!first_point_set && input.mouse_button_down(1)) {
            glm::vec2 mouse_pos = input.mouse_world_pos();
            for (auto& p : last.points) {
                p = mouse_pos;
            }
            last.update_render_data();
            first_point_set = true;
            return;
        }

        last.points[3] = input.mouse_world_pos();

        glm::vec2 start_to_end = last.points[3] - last.points[0];
        last.points[1] = last.points[0] + start_to_end * 0.3f;
        last.points[2] = last.points[3] + start_to_end * 0.3f;
        last.update_render_data();

        if (input.mouse_button_down(1)) {
            creating_new_spline = false;
            first_point_set = false;
        }
        return;
    }

    if (input.mouse_button_up(1)) {
        selected_point = nullptr;
        return;
    }

    glm::vec2 mouse_pos = input.mouse_world_pos();
    if (input.mouse_button_down(1)) {
        for (size_t i = 0; i < splines.size(); ++i) {
            for (auto& p : splines[i].points) {
                if (glm::length(p - mouse_pos) < 4.0f) {
                    selected_point = &p;
                    selected_spline_index = i;
                }
            }
        }
    }

    if (selected_point) {
        *selected_point = mouse_pos;
        splines[selected_spline_index].update_render_data();
    }
}

void SplineEditor::update_gui() {
    using namespace ImGui;

    Begin("Spline Editor");
    Text("Splines");

    char** spline_names = new char*[splines.size()];

    for (size_t i = 0; i < splines.size(); ++i) {
        spline_names[i] = new char[16];
        sprintf_s(spline_names[i], 16, "Spline %zu", i);
    }

    int selected = static_cast<int>(selected_spline_index);
    ListBox("", &selected, spline_names, static_cast<int>(splines.size()));

    selected_spline_index = static_cast<size_t>(selected);

    for (size_t i = 0; i < splines.size(); ++i) {
        delete[] spline_names[i];
    }
    delete[] spline_names;

    if (Button("New spline") && !creating_new_spline) {
        glm::vec2 points[4] = {glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f),
                               glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f)};
        splines.push_back(Spline(points));
        creating_new_spline = true;
        selected_spline_index = splines.size() - 1;
    }
    SameLine();

    if (Button("Delete")) {
        splines.erase(splines.begin() + selected_spline_index);
    }

    End();
}

void SplineEditor::render(GLuint shader_id, GLuint color_uniform_loc) {
    glUseProgram(shader_id);

    size_t num_splines_to_render = splines.size();
    if (creating_new_spline && !first_point_set)
        num_splines_to_render -= 1;

    for (size_t i = 0; i < num_splines_to_render; ++i) {
        glUniform4f(color_uniform_loc, 0.0f, 1.0f, 0.0f,
                    1.0f); // Green

        glLineWidth(1.0f);
        splines[i].line_vao.draw(GL_LINE_STRIP);
    }

    if (selected_spline_index >= 0 && selected_spline_index < splines.size() &&
        (!creating_new_spline && !first_point_set)) {
        glUniform4f(color_uniform_loc, 0.7f, 0.0f, 0.7f,
                    1.0f); // Light purple
        splines[selected_spline_index].point_vao.draw(GL_LINES);

        glUniform4f(color_uniform_loc, 1.0f, 0.0f, 1.0f,
                    1.0f); // Purple
        splines[selected_spline_index].point_vao.draw(GL_POINTS);
    }
}