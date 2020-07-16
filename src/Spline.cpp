#pragma once
#include "pch.h"
#include "Spline.h"
#include "Game.h"
#include "Util.h"
// #include <shobjidl.h>

/////                                   /////
/////               Spline              /////
/////                                   /////

void Spline::init(glm::vec2 points_[NUM_POINTS]) {
    if (points_ != nullptr) {
        memcpy_s(points, 4 * sizeof(glm::vec2), points_, 4 * sizeof(glm::vec2));
    } else {
        points[0] = glm::vec2(0.0f, 0.0f);
        points[1] = glm::vec2(0.0f, 0.0f);
        points[2] = glm::vec2(0.0f, 0.0f);
        points[3] = glm::vec2(0.0f, 0.0f);
    }

    // Init line render data
    parameter_matrix = glm::mat4(
        glm::vec4(points[P1], 0.0f, 1.0f), glm::vec4(points[P2], 0.0f, 1.0f),
        glm::vec4(points[T1] - points[P1], 0.0f, 1.0f),
        glm::vec4(points[P2] - points[T2], 0.0f, 1.0f));

    if (vertices_initialized) {
        update_render_data();
    } else {
        GLuint indices[RENDER_STEPS];
        glm::vec4 interpolation_vector;
        for (size_t i = 0; i < RENDER_STEPS; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(RENDER_STEPS);
            interpolation_vector = {t * t * t, t * t, t, 1.0f};
            line_shader_vertices[i].pos =
                parameter_matrix * hermite_matrix * interpolation_vector;

            indices[i] = static_cast<GLuint>(i);
        }
        line_vao.init(indices, RENDER_STEPS, line_shader_vertices.data(),
                      static_cast<GLuint>(line_shader_vertices.size()));

        // Init point render data
        for (size_t i = 0; i < NUM_POINTS; ++i) {
            point_shader_vertices[i].pos = glm::vec4(points[i], 0.0f, 1.0f);
        }
        point_vao.init(indices, NUM_POINTS, point_shader_vertices.data(),
                       static_cast<GLuint>(point_shader_vertices.size()));

        vertices_initialized = true;
    }
}

void Spline::update_render_data() {
    // Line
    // @optimize
    parameter_matrix = glm::mat4(
        glm::vec4(points[P1], 0.0f, 1.0f), glm::vec4(points[P2], 0.0f, 1.0f),
        glm::vec4(points[T1] - points[P1], 0.0f, 1.0f),
        glm::vec4(points[P2] - points[T2], 0.0f, 1.0f));

    for (size_t i = 0; i < RENDER_STEPS; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(RENDER_STEPS - 1);

        line_shader_vertices[i].pos = get_point_on_spline(t);
    }

    line_vao.update_vertex_data(
        line_shader_vertices.data(),
        static_cast<GLuint>(line_shader_vertices.size()));

    // Points
    for (size_t i = 0; i < NUM_POINTS; ++i) {
        point_shader_vertices[i].pos = glm::vec4(points[i], 0.0f, 1.0f);
    }
    point_vao.update_vertex_data(
        point_shader_vertices.data(),
        static_cast<GLuint>(point_shader_vertices.size()));
}

glm::vec2 Spline::get_point_on_spline(float t) const {
    glm::vec4 interpolation_vector = {t * t * t, t * t, t, 1.0f};
    return static_cast<glm::vec2>(parameter_matrix * hermite_matrix *
                                  interpolation_vector);
}

/////                                   /////
/////           SplineEditor            /////
/////                                   /////

// Returns index of backward/forward spline that's the coutnerpart to the spline
// with spline_index
static size_t opposite_direction(size_t spline_index) {
    size_t result;
    if (spline_index % 2 == 0) {
        result = spline_index + 1;
    } else {
        result = spline_index - 1;
    }
    return result;
}

void SplineEditor::save_splines(bool get_new_file_path) {
    if (get_new_file_path || save_path == nullptr) {
        bool success = !get_save_path(save_path, L".spl", L"*.spl", L"spl");
        SDL_assert_always(success);
    }

    // Write data
    SDL_RWops* file = SDL_RWFromFile(save_path, "wb");

    SDL_RWwrite(file, &num_animations, sizeof(num_animations), 1);

    for (size_t n_anim = 0; n_anim < num_animations; ++n_anim) {
        auto& anim = animations[n_anim];
        SDL_RWwrite(file, anim.name, sizeof(char), Animation::MAX_NAME_LENGTH);

        for (auto& spline : anim.splines) {
            SDL_RWwrite(file, spline.points, sizeof(*spline.points),
                        Spline::NUM_POINTS);
        }
    }

    SDL_RWclose(file);
}

void SplineEditor::load_splines(const char* path) {
    if (path == nullptr) {
        bool success = get_load_path(save_path, L".spl", L"*.spl");
        SDL_assert_always(success);
    } else {
        if (save_path) {
            delete[] save_path;
        }
        size_t length = strlen(path) + 1;
        save_path = new char[length];
        strcpy_s(save_path, length, path);
    }

    // Read data
    SDL_RWops* file = SDL_RWFromFile(save_path, "rb");

    SDL_RWread(file, &num_animations, sizeof(num_animations), 1);

    for (size_t n_anim = 0; n_anim < num_animations; ++n_anim) {
        auto& anim = animations[n_anim];
        SDL_RWread(file, &anim, sizeof(char), Animation::MAX_NAME_LENGTH);

        glm::vec2 point_buf[Spline::NUM_POINTS];
        for (auto& spline : anim.splines) {
            SDL_RWread(file, point_buf, sizeof(*point_buf), Spline::NUM_POINTS);
            spline.init(point_buf);
        }
    }

    SDL_RWclose(file);
}

void SplineEditor::init(const Entity* parent_, Animation* animations_,
                        const Bone* limb_bones_[4][2],
                        const char* spline_path) {
    SDL_assert(parent_ != nullptr);
    parent = parent_;
    animations = animations_;

    size_t path_length = strnlen_s(spline_path, MAX_SAVE_PATH_LENGTH) + 1;

    if (save_path)
        delete[] save_path;
    save_path = new char[path_length];

    strcpy_s(save_path, path_length, spline_path);

    for (size_t i = 0; i < 4; ++i) {
        limb_bones[i][0] = limb_bones_[i][0];
        limb_bones[i][1] = limb_bones_[i][1];
    }

    load_splines(spline_path);

    // Init render data for rendering circle
    GLuint circle_indices[CIRCLE_SEGMENTS];
    for (size_t i = 0; i < CIRCLE_SEGMENTS; ++i) {
        circle_indices[i] = static_cast<GLuint>(i);
    }

    circle_vao.init(circle_indices, CIRCLE_SEGMENTS, nullptr, CIRCLE_SEGMENTS);
}

void SplineEditor::set_spline_point(glm::vec2 new_point, size_t point_index,
                                    size_t spline_index) {
    if (point_index == -1) {
        SDL_assert(selected_point_index != -1);
        point_index = selected_point_index;
    }
    if (spline_index == -1) {
        SDL_assert(selected_spline_index != -1);
        spline_index = selected_spline_index;
    }

    Animation& anim = animations[selected_animation_index];

    anim.splines[spline_index].points[point_index] = new_point;
    // @OPTIMIZATION: Is called way more often than it needs to be
    anim.splines[spline_index].update_render_data();

    { // Conditionally set points on other splines as well
        bool updated = false;
        auto& partner_spline = anim.splines[opposite_direction(spline_index)];
        if (connect_point_pairs &&
            (point_index == Spline::P1 || point_index == Spline::P2)) {
            partner_spline.points[3 - point_index] = new_point;
            updated = true;
        }
        if (connect_tangent_pairs &&
            (point_index == Spline::T1 || point_index == Spline::T2)) {
            partner_spline.points[3 - point_index] = new_point;
            updated = true;
        }

        if (updated)
            partner_spline.update_render_data();
    }

    // Same for other limb on other side
    if (mirror_to_opposing_limb) {
        size_t mirror_limb_spline_index;
        if (spline_index < 2 || (spline_index >= 4 && spline_index < 6)) {
            mirror_limb_spline_index = spline_index + 2;
        } else {
            mirror_limb_spline_index = spline_index - 2;
        }

        glm::vec2 mirrored_limb_point =
            new_point +
            glm::vec2(limb_bones[mirror_limb_spline_index / 2][0]
                          ->bind_pose_transform[2] -
                      limb_bones[spline_index / 2][0]->bind_pose_transform[2]);

        // Does the same as above, but with mirror_limb_spline_index instead of
        // spline_index
        anim.splines[mirror_limb_spline_index].points[point_index] =
            mirrored_limb_point;
        anim.splines[mirror_limb_spline_index].update_render_data();

        // Conditionally set points on other splines as well
        bool updated = false;
        auto& other_partner_spline =
            anim.splines[opposite_direction(mirror_limb_spline_index)];
        if (connect_point_pairs &&
            (point_index == Spline::P1 || point_index == Spline::P2)) {
            other_partner_spline.points[3 - point_index] = mirrored_limb_point;
            updated = true;
        }
        if (connect_tangent_pairs &&
            (point_index == Spline::T1 || point_index == Spline::T2)) {
            other_partner_spline.points[3 - point_index] = mirrored_limb_point;
            updated = true;
        }

        if (updated)
            other_partner_spline.update_render_data();
    }
}

bool SplineEditor::update(const MouseKeyboardInput& input) {
    // This value is returned at the end
    bool keep_open = update_gui();

    glm::vec2 mouse_pos =
        parent->world_to_local_space(glm::vec3(input.mouse_pos_world(), 0.0f));

    if (creating_new_spline) {
        // Set the points and return
        SDL_assert_always(selected_animation_index < num_animations &&
                          selected_spline_index < Animation::NUM_SPLINES);

        Spline& selected_spline =
            animations[selected_animation_index].splines[selected_spline_index];

        if (!first_point_set && input.mouse_button_down(MouseButton::LEFT)) {
            // Set all points to mouse position
            for (size_t i = 0; i < Spline::NUM_POINTS; ++i) {
                set_spline_point(mouse_pos, i);
            }
            first_point_set = true;

        } else {
            // Set P2 to mouse position and the others to points along the
            // way from P1 to P2 and return
            set_spline_point(mouse_pos, Spline::P2);
            glm::vec2 start_to_end = selected_spline.points[Spline::P2] -
                                     selected_spline.points[Spline::P1];
            set_spline_point(selected_spline.points[Spline::P1] +
                                 start_to_end * 0.3f,
                             Spline::T1);
            set_spline_point(selected_spline.points[Spline::P2] -
                                 start_to_end * 0.3f,
                             Spline::T2);

            if (input.mouse_button_down(MouseButton::LEFT)) {
                creating_new_spline = false;
                first_point_set = false;
            }
        }
    } else if (input.mouse_button_up(MouseButton::LEFT)) {
        // No point selected anymore
        selected_point_index = static_cast<size_t>(-1);

    } else if (!(selected_spline_index >= Animation::NUM_SPLINES ||
                 selected_animation_index >= num_animations)) {
        // A valid animation/spline is selected
        auto& spline =
            animations[selected_animation_index].splines[selected_spline_index];

        if (input.mouse_button_down(MouseButton::LEFT)) {
            // If a point was clicked, select it
            for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
                if (glm::length(spline.points[n_point] - mouse_pos) < 0.1f) {
                    selected_point_index = n_point;
                }
            }
        } else if (selected_point_index < Spline::NUM_POINTS) {
            // Move the selected point (and it's tangent, if it's P1/P2) to the
            // mouse position
            if (selected_point_index == Spline::P1) {
                glm::vec2 move = mouse_pos - spline.points[Spline::P1];
                set_spline_point(spline.points[Spline::T1] + move, Spline::T1);

            } else if (selected_point_index == Spline::P2) {
                glm::vec2 move = mouse_pos - spline.points[Spline::P2];
                set_spline_point(spline.points[Spline::T2] + move, Spline::T2);
            }

            set_spline_point(mouse_pos);
        }
    }

    return keep_open;
}

bool SplineEditor::update_gui() {
    bool keep_open = true;

    using namespace ImGui;

    Begin("Spline Editor", &keep_open);
    Text("Animatons");

    const char** animation_names = new const char*[num_animations];
    for (size_t i = 0; i < num_animations; ++i) {
        animation_names[i] = animations[i].name;
    }

    int selected = static_cast<int>(selected_animation_index);
    ListBox("Animations", &selected, animation_names,
            static_cast<int>(num_animations), static_cast<int>(num_animations));
    selected_animation_index = static_cast<size_t>(selected);

    delete[] animation_names;

    NewLine();
    Text("Splines");

    const char* spline_names[Animation::NUM_SPLINES] = {
        "Left_Arm_Forward",   "Left_Arm_Backward", "Right_Arm_Forward",
        "Right_Arm_Backward", "Left_Leg_Forward",  "Left_Leg_Backward",
        "Right_Leg_Forward",  "Right_Leg_Backward"};

    selected = static_cast<int>(selected_spline_index);
    ListBox("Splines", &selected, spline_names,
            static_cast<int>(Animation::NUM_SPLINES),
            static_cast<int>(Animation::NUM_SPLINES));
    selected_spline_index = static_cast<size_t>(selected);

    if (Button("Replace with new spline") && !creating_new_spline) {
        SDL_assert_always(selected_animation_index < num_animations &&
                          selected_spline_index < Animation::NUM_SPLINES);

        // Just set this, update() handles the rest
        creating_new_spline = true;
        first_point_set = false;
    }

    if (Button("Open...")) {
        load_splines();
    }
    SameLine();
    if (Button("Save")) {
        save_splines();
    }
    SameLine();
    if (Button("Save as...")) {
        save_splines(true);
    }

    NewLine();
    Checkbox("Connect point pairs", &connect_point_pairs);
    Checkbox("Connect tangent pairs", &connect_tangent_pairs);
    Checkbox("Mirror to opposing limb", &mirror_to_opposing_limb);

    if (selected_spline_index < Animation::NUM_SPLINES) {
        const float sensitivity = 0.1f;

        size_t forward_spline_index = selected_spline_index;
        forward_spline_index -= forward_spline_index % 2;

        glm::vec2* points = animations[selected_animation_index]
                                .splines[forward_spline_index]
                                .points;

        // NOTE: Calling set_spline_point ever time might be inefficient,
        // but it's a big hassle otherwise
        if (DragFloat2("P1 (forward)", value_ptr(points[0]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::P1], Spline::P1,
                             forward_spline_index);
        }
        if (DragFloat2("T1 (forward)", value_ptr(points[1]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::T1], Spline::T1,
                             forward_spline_index);
        }
        if (DragFloat2("T2 (forward)", value_ptr(points[2]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::T2], Spline::T2,
                             forward_spline_index);
        }
        if (DragFloat2("P2 (forward)", value_ptr(points[3]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::P2], Spline::P2,
                             forward_spline_index);
        }

        NewLine();

        size_t backward_spline_index = forward_spline_index + 1;
        points = animations[selected_animation_index]
                     .splines[backward_spline_index]
                     .points;

        if (DragFloat2("P1 (backward)", value_ptr(points[0]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::P1], Spline::P1,
                             backward_spline_index);
        }
        if (DragFloat2("T1 (backward)", value_ptr(points[1]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::T1], Spline::T1,
                             backward_spline_index);
        }
        if (DragFloat2("T2 (backward)", value_ptr(points[2]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::T2], Spline::T2,
                             backward_spline_index);
        }
        if (DragFloat2("P2 (backward)", value_ptr(points[3]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::P2], Spline::P2,
                             backward_spline_index);
        }
    }

    End();

    return keep_open;
}

void SplineEditor::render(const Renderer& renderer, bool spline_edit_mode) {
    renderer.debug_shader.use();
    glLineWidth(1.0f);

    // Draw circle for selected limb
    if (selected_spline_index < Animation::NUM_SPLINES && spline_edit_mode) {
        const Bone** bones = limb_bones[selected_spline_index / 2];

        // @OPTIMIZATION: Calculate this stuff less often
        glm::vec2 limb_root_position = static_cast<glm::vec2>(
            bones[0]->get_transform() * bones[0]->bind_pose_transform[2]);
        float radius = bones[0]->length + bones[1]->length;

        DebugShader::Vertex circle_vertices[CIRCLE_SEGMENTS];

        for (size_t n_segment = 0; n_segment < CIRCLE_SEGMENTS; ++n_segment) {
            float theta = static_cast<float>(n_segment) /
                          static_cast<float>(CIRCLE_SEGMENTS) * 2.0f * PI;

            circle_vertices[n_segment].pos =
                limb_root_position +
                glm::vec2(radius * cosf(theta), radius * sinf(theta));
        }

        renderer.debug_shader.set_color(&Color::LIGHT_BLUE);
        circle_vao.update_vertex_data(circle_vertices, CIRCLE_SEGMENTS);
        circle_vao.draw(GL_LINE_LOOP);
    }

    // Draw splines
    renderer.debug_shader.set_color(&Color::GREEN);

    if (renderer.draw_all_splines) {
        // Draw all splines (except selected)
        for (size_t n_anim = 0; n_anim < num_animations; ++n_anim) {
            for (size_t n_spline = 0; n_spline < Animation::NUM_SPLINES;
                 ++n_spline) {

                if (n_anim == selected_animation_index &&
                    n_spline == selected_spline_index && creating_new_spline &&
                    !first_point_set) {
                    continue;
                }

                animations[n_anim].splines[n_spline].line_vao.draw(
                    GL_LINE_STRIP);
            }
        }
    }

    if (selected_animation_index < num_animations &&
        selected_spline_index < Animation::NUM_SPLINES &&
        !(creating_new_spline && !first_point_set)) {
        // Draw selected spline (with points)
        auto& spline =
            animations[selected_animation_index].splines[selected_spline_index];
        spline.line_vao.draw(GL_LINE_STRIP);

        if (spline_edit_mode) {
            renderer.debug_shader.set_color(&Color::LIGHT_PURPLE);
            spline.point_vao.draw(GL_LINES);

            renderer.debug_shader.set_color(&Color::PURPLE);
            spline.point_vao.draw(GL_POINTS);
        }
    }
}