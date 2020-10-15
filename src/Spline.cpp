#pragma once
#include "pch.h"
#include "Spline.h"
#include "Game.h"
#include "Util.h"

/////                                   /////
/////               Spline              /////
/////                                   /////

void Spline::init(const glm::vec2 points_[NUM_POINTS]) {
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
        glm::vec4(points[T1], 0.0f, 1.0f), glm::vec4(points[T2], 0.0f, 1.0f));

    if (vertices_initialized) {
        update_render_data();
    } else {
        GLuint indices[RENDER_STEPS];
        glm::vec4 interpolation_vector;
        for (size_t i = 0; i < RENDER_STEPS; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(RENDER_STEPS);
            interpolation_vector = {t * t * t, t * t, t, 1.0f};
            line_shader_vertices[i].pos =
                parameter_matrix * HERMITE_MATRIX * interpolation_vector;

            indices[i] = static_cast<GLuint>(i);
        }
        line_vao.init(indices, RENDER_STEPS, line_shader_vertices.data(),
                      static_cast<GLuint>(line_shader_vertices.size()));

        // Init point render data
        point_shader_vertices[P1].pos = glm::vec4(points[P1], 0.0f, 1.0f);
        point_shader_vertices[T1].pos =
            glm::vec4(points[T1] - points[P1], 0.0f, 1.0f);
        point_shader_vertices[T2].pos =
            glm::vec4(points[T2] - points[P2], 0.0f, 1.0f);
        point_shader_vertices[P2].pos = glm::vec4(points[P2], 0.0f, 1.0f);

        point_vao.init(indices, NUM_POINTS, point_shader_vertices.data(),
                       static_cast<GLuint>(point_shader_vertices.size()));

        vertices_initialized = true;
    }
}

void Spline::render(const Renderer& renderer) const {
    renderer.debug_shader.use();
    renderer.debug_shader.set_color(&Color::GREEN);

    line_vao.draw(GL_LINE_STRIP);
}

const glm::vec2& Spline::get_point(PointName p) const { return points[p]; }

const glm::vec2* Spline::get_points() const { return points; }

void Spline::set_points(const glm::vec2 new_points[NUM_POINTS]) {
    for (size_t i = 0; i < NUM_POINTS; ++i) {
        points[i] = new_points[i];
    }
    update_render_data();
}

void Spline::set_point(PointName name, glm::vec2 point) {
    if (name == T1 || name == T2) {
        points[name] = point;
        return;
    }

    glm::vec2 point_delta = point - points[name];
    points[name] = point;
    if (name == P1) {
        points[T1] += point_delta;
    } else {
        points[T2] += point_delta;
    }
    update_render_data();
}

void Spline::update_render_data() {
    SDL_assert(vertices_initialized);
    // Line
    // @optimize
    parameter_matrix = glm::mat4(
        glm::vec4(points[P1], 0.0f, 1.0f), glm::vec4(points[P2], 0.0f, 1.0f),
        glm::vec4(points[T1], 0.0f, 1.0f), glm::vec4(points[T2], 0.0f, 1.0f));

    for (size_t i = 0; i < RENDER_STEPS; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(RENDER_STEPS - 1);

        line_shader_vertices[i].pos = get_point_on_spline(t);
    }

    line_vao.update_vertex_data(
        line_shader_vertices.data(),
        static_cast<GLuint>(line_shader_vertices.size()));

    // Points
    point_shader_vertices[P1].pos = glm::vec4(points[P1], 0.0f, 1.0f);
    point_shader_vertices[T1].pos =
        glm::vec4(points[P1] + points[T1], 0.0f, 1.0f);
    point_shader_vertices[T2].pos =
        glm::vec4(points[P2] + points[T2], 0.0f, 1.0f);
    point_shader_vertices[P2].pos = glm::vec4(points[P2], 0.0f, 1.0f);

    point_vao.update_vertex_data(
        point_shader_vertices.data(),
        static_cast<GLuint>(point_shader_vertices.size()));
}

glm::vec2 Spline::get_point_on_spline(float t) const {
    glm::vec4 interpolation_vector = {t * t * t, t * t, t, 1.0f};
    return static_cast<glm::vec2>(parameter_matrix * HERMITE_MATRIX *
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
    if (get_new_file_path || save_path.empty()) {
        bool success = get_save_path(save_path, L".spl", L"*.spl", L"spl");
        SDL_assert_always(success);
    }

    // Construct an array of all the spline points
    const size_t num_splines_total = NUM_ANIMATIONS * NUM_SPLINES_PER_ANIMATION;
    glm::vec2 all_points[num_splines_total * Spline::NUM_POINTS];

    size_t num_copied = 0;
    for (size_t n_spline = 0; n_spline < NUM_SPLINES_PER_ANIMATION;
         ++n_spline) {
        for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
            all_points[num_copied++] =
                spline_set->walk[n_spline].points[n_point];
        }
    }
    for (size_t n_spline = 0; n_spline < NUM_SPLINES_PER_ANIMATION;
         ++n_spline) {
        for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
            all_points[num_copied++] =
                spline_set->run[n_spline].points[n_point];
        }
    }
    for (size_t n_spline = 0; n_spline < NUM_SPLINES_PER_ANIMATION;
         ++n_spline) {
        for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
            all_points[num_copied++] =
                spline_set->idle[n_spline].points[n_point];
        }
    }
    SDL_assert(num_copied == num_splines_total * Spline::NUM_POINTS);

    // Write data
    size_t num_bytes_to_write = sizeof(all_points);
    SDL_RWops* file = SDL_RWFromFile(save_path.c_str(), "wb");
    size_t num_bytes_written =
        SDL_RWwrite(file, all_points, 1, num_bytes_to_write);
    SDL_RWclose(file);

    if (num_bytes_written != num_bytes_to_write) {
        printf("Error writing %s: %s", save_path.c_str(), SDL_GetError());
        SDL_TriggerBreakpoint();
    }
}

void SplineEditor::load_splines(const std::string& path) {
    if (path.empty()) {
        bool success = get_load_path(save_path, L".spl", L"*.spl");
        SDL_assert_always(success);
    }

    // Read data
    const size_t num_splines_total = NUM_ANIMATIONS * NUM_SPLINES_PER_ANIMATION;
    glm::vec2 all_points[num_splines_total * Spline::NUM_POINTS];

    size_t num_bytes_to_read = sizeof(all_points);
    SDL_RWops* file = SDL_RWFromFile(save_path.c_str(), "rb");
    size_t num_bytes_read = SDL_RWread(file, all_points, 1, num_bytes_to_read);
    SDL_RWclose(file);

    if (num_bytes_read != num_bytes_to_read) {
        printf("Error reading %s: %s", save_path.c_str(), SDL_GetError());
        SDL_TriggerBreakpoint();
    }

    // Initialize the splines from the point data
    size_t num_copied = 0;
    for (size_t n_spline = 0; n_spline < NUM_SPLINES_PER_ANIMATION;
         ++n_spline) {
        spline_set->walk[n_spline].init(&all_points[num_copied]);
        num_copied += Spline::NUM_POINTS;
    }
    for (size_t n_spline = 0; n_spline < NUM_SPLINES_PER_ANIMATION;
         ++n_spline) {
        spline_set->run[n_spline].init(&all_points[num_copied]);
        num_copied += Spline::NUM_POINTS;
    }
    for (size_t n_spline = 0; n_spline < NUM_SPLINES_PER_ANIMATION;
         ++n_spline) {
        spline_set->idle[n_spline].init(&all_points[num_copied]);
        num_copied += Spline::NUM_POINTS;
    }
    SDL_assert(num_copied == num_splines_total * Spline::NUM_POINTS);
}

void SplineEditor::init(const Entity* parent_, SplineSet* splines_,
                        Limb* limbs_, const char* spline_path) {
    SDL_assert(parent_ != nullptr);
    parent = parent_;
    spline_set = splines_;
    limbs = limbs_;

    save_path = std::string{spline_path};

    load_splines(spline_path);

    // Init render data for rendering circle
    GLuint circle_indices[CIRCLE_SEGMENTS];
    for (size_t i = 0; i < CIRCLE_SEGMENTS; ++i) {
        circle_indices[i] = static_cast<GLuint>(i);
    }

    circle_vao.init(circle_indices, CIRCLE_SEGMENTS, nullptr, CIRCLE_SEGMENTS);
}

void SplineEditor::set_spline_point(glm::vec2 new_point, size_t point_index,
                                    size_t spline_index,
                                    bool absolute_tangent) {
    if (point_index == -1) {
        SDL_assert(selected_point_index != -1);
        point_index = selected_point_index;
    }
    if (spline_index == -1) {
        SDL_assert(selected_spline_index != -1);
        spline_index = selected_spline_index;
    }

    Spline* splines = nullptr;
    if (selected_animation == WALK) {
        splines = spline_set->walk;
    } else if (selected_animation == RUN) {
        splines = spline_set->run;
    } else if (selected_animation == IDLE) {
        splines = spline_set->idle;
    } else {
        SDL_TriggerBreakpoint();
    }

    auto& selected_spline = splines[spline_index];
    auto& point_to_set = selected_spline.points[point_index];

    if (absolute_tangent && point_index == Spline::T1) {
        point_to_set = new_point - selected_spline.points[Spline::P1];
    } else if (absolute_tangent && point_index == Spline::T2) {
        point_to_set = new_point - selected_spline.points[Spline::P2];
    } else {
        point_to_set = new_point;
    }
    // @OPTIMIZATION: Is called way more often than it needs to be
    selected_spline.update_render_data();

    // Conditionally set points on other splines as well
    bool updated = false;
    auto& partner_spline = splines[opposite_direction(spline_index)];
    if (connect_point_pairs &&
        (point_index == Spline::P1 || point_index == Spline::P2)) {
        partner_spline.points[3 - point_index] = point_to_set;
        updated = true;
    }
    if (connect_tangent_pairs &&
        (point_index == Spline::T1 || point_index == Spline::T2)) {
        partner_spline.points[3 - point_index] = point_to_set;
        updated = true;
    }

    if (updated)
        partner_spline.update_render_data();
}

bool SplineEditor::update(const MouseKeyboardInput& input) {
    // This value is returned at the end
    bool keep_open = update_gui();

    glm::vec2 mouse_pos = parent->world_to_local_space(input.mouse_pos_world());

    if (creating_new_spline) {
        // Set the points
        SDL_assert_always(selected_animation != NONE &&
                          selected_spline_index < NUM_SPLINES_PER_ANIMATION);

        Spline* splines = nullptr;
        if (selected_animation == WALK) {
            splines = spline_set->walk;
        } else if (selected_animation == RUN) {
            splines = spline_set->run;
        } else if (selected_animation == IDLE) {
            splines = spline_set->idle;
        } else {
            SDL_TriggerBreakpoint();
        }

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
            auto& selected_spline = splines[selected_spline_index];

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

    } else if (selected_spline_index < NUM_SPLINES_PER_ANIMATION ||
               selected_animation != NONE) {
        // A valid animation/spline is selected
        Spline* spline = nullptr;
        if (selected_animation == WALK) {
            spline = &spline_set->walk[selected_spline_index];
        } else if (selected_animation == RUN) {
            spline = &spline_set->run[selected_spline_index];
        } else if (selected_animation == IDLE) {
            spline = &spline_set->idle[selected_spline_index];
        } else {
            SDL_TriggerBreakpoint();
        }

        if (input.mouse_button_down(MouseButton::LEFT)) {
            // If a point was clicked, select it
            if (glm::length(spline->points[Spline::P1] - mouse_pos) < 0.1f) {
                selected_point_index = Spline::P1;

            } else if (glm::length(spline->points[Spline::P2] - mouse_pos) <
                       0.1f) {
                selected_point_index = Spline::P2;

            } else if (glm::length(spline->points[Spline::P1] +
                                   spline->points[Spline::T1] - mouse_pos) <
                       0.1f) {
                selected_point_index = Spline::T1;

            } else if (glm::length(spline->points[Spline::P2] +
                                   spline->points[Spline::T2] - mouse_pos) <
                       0.1f) {
                selected_point_index = Spline::T2;
            }

        } else if (selected_point_index < Spline::NUM_POINTS) {
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

    const char* animation_names[NUM_ANIMATIONS] = {"Walk", "Run", "Idle"};

    int selected = static_cast<int>(selected_animation);
    ListBox("Animations", &selected, animation_names,
            static_cast<int>(NUM_ANIMATIONS), static_cast<int>(NUM_ANIMATIONS));
    SDL_assert(selected > -1 && selected <= NUM_ANIMATIONS);
    selected_animation = static_cast<SelectedAnimation>(selected);

    NewLine();
    Text("Splines");

    const char* spline_names[NUM_SPLINES_PER_ANIMATION] = {
        "Leg_Forward", "Leg_Backward", "Arm_Forward", "Arm_Backward", "Pelvis"};

    selected = static_cast<int>(selected_spline_index);
    ListBox("Splines", &selected, spline_names,
            static_cast<int>(NUM_SPLINES_PER_ANIMATION),
            static_cast<int>(NUM_SPLINES_PER_ANIMATION));
    selected_spline_index = static_cast<size_t>(selected);

    if (Button("Replace with new spline") && !creating_new_spline) {
        // @CLEANUP: Is this necessary?
        SDL_assert_always(selected_animation < NUM_ANIMATIONS &&
                          selected_spline_index < NUM_SPLINES_PER_ANIMATION);

        // Just set this, update() handles the rest
        creating_new_spline = true;
        first_point_set = false;
    }

    if (Button("Open...")) {
        load_splines(save_path);
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

    if (selected_spline_index < NUM_SPLINES_PER_ANIMATION) {
        const float sensitivity = 0.1f;

        size_t forward_spline_index = selected_spline_index;
        forward_spline_index -= forward_spline_index % 2;

        glm::vec2* points = nullptr;
        if (selected_animation == WALK) {
            points = spline_set->walk[forward_spline_index].points;
        } else if (selected_animation == RUN) {
            points = spline_set->run[forward_spline_index].points;
        } else if (selected_animation == IDLE) {
            points = spline_set->idle[forward_spline_index].points;
        } else {
            SDL_TriggerBreakpoint();
        }

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
                             forward_spline_index, false);
        }
        if (DragFloat2("T2 (forward)", value_ptr(points[2]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::T2], Spline::T2,
                             forward_spline_index, false);
        }
        if (DragFloat2("P2 (forward)", value_ptr(points[3]), sensitivity, 0.0f,
                       0.0f, "% .2f")) {
            set_spline_point(points[Spline::P2], Spline::P2,
                             forward_spline_index);
        }

        NewLine();

        size_t backward_spline_index = forward_spline_index + 1;
        if (selected_animation == WALK) {
            spline_set->walk[backward_spline_index].points;
        } else if (selected_animation == RUN) {
            spline_set->run[backward_spline_index].points;
        } else if (selected_animation == IDLE) {
            spline_set->idle[backward_spline_index].points;
        }

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

    if (selected_spline_index < NUM_SPLINES_PER_ANIMATION && spline_edit_mode) {
        // Draw circle for selected limb
        renderer.debug_shader.set_model(&parent->get_model_matrix());
        renderer.debug_shader.set_color(&Color::LIGHT_BLUE);

        // @OPTIMIZATION: Calculate this stuff less often
        float radius;
        if (selected_spline_index == Animator::LEG_FORWARD ||
            selected_spline_index == Animator::LEG_BACKWARD) {

            radius = limbs[Animator::LEFT_LEG].length();
        } else {
            radius = limbs[Animator::LEFT_ARM].length();
        }

        DebugShader::Vertex circle_vertices[CIRCLE_SEGMENTS];

        for (size_t n_segment = 0; n_segment < CIRCLE_SEGMENTS; ++n_segment) {
            float theta = static_cast<float>(n_segment) /
                          static_cast<float>(CIRCLE_SEGMENTS) * 2.0f * PI;

            circle_vertices[n_segment].pos =
                glm::vec2(radius * cosf(theta), radius * sinf(theta));
        }

        circle_vao.update_vertex_data(circle_vertices, CIRCLE_SEGMENTS);
        circle_vao.draw(GL_LINE_LOOP);
    }

    // Draw splines
    renderer.debug_shader.set_color(&Color::GREEN);

    if (selected_animation != NONE &&
        selected_spline_index < NUM_SPLINES_PER_ANIMATION &&
        !(creating_new_spline && !first_point_set)) {
        // Draw selected spline (with points)
        const Spline* spline = nullptr;
        if (selected_animation == WALK) {
            spline = &spline_set->walk[selected_spline_index];
        } else if (selected_animation == RUN) {
            spline = &spline_set->run[selected_spline_index];
        } else if (selected_animation == IDLE) {
            spline = &spline_set->idle[selected_spline_index];
        } else {
            SDL_TriggerBreakpoint();
        }
        spline->line_vao.draw(GL_LINE_STRIP);

        if (spline_edit_mode) {
            renderer.debug_shader.set_color(&Color::LIGHT_PURPLE);
            spline->point_vao.draw(GL_LINES);

            renderer.debug_shader.set_color(&Color::PURPLE);
            spline->point_vao.draw(GL_POINTS);
        }
    }
}