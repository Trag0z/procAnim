#pragma once
#include "pch.h"
#include "Spline.h"
#include "Game.h"
#include <shobjidl.h>

/////                                   /////
/////               Spline              /////
/////                                   /////

void Spline::init(glm::vec2 points_[num_points]) {
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
    // @optimize
    parameter_matrix = glm::mat4(
        glm::vec4(points[0], 0.0f, 1.0f), glm::vec4(points[3], 0.0f, 1.0f),
        glm::vec4(points[1], 0.0f, 1.0f), glm::vec4(points[2], 0.0f, 1.0f));

    for (size_t i = 0; i < render_steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(render_steps - 1);

        line_shader_vertices[i].pos = get_point_on_spline(t);
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

glm::vec4 Spline::get_point_on_spline(float t) const {
    glm::vec4 interpolation_vector = {t * t * t, t * t, t, 1.0f};
    return parameter_matrix * hermite_matrix * interpolation_vector;
}

/////                                   /////
/////           SplineEditor            /////
/////                                   /////

void SplineEditor::save_splines(bool get_new_file_path) {
    if (get_new_file_path || save_path == nullptr) {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                                              COINIT_DISABLE_OLE1DDE);

        SDL_assert_always(SUCCEEDED(hr));
        IFileSaveDialog* pFileSave;

        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                              IID_IFileSaveDialog,
                              reinterpret_cast<void**>(&pFileSave));

        SDL_assert_always(SUCCEEDED(hr));

        COMDLG_FILTERSPEC file_type = {L".spl", L"*.spl"};
        pFileSave->SetFileTypes(1, &file_type);

        // Show the Open dialog box.
        hr = pFileSave->Show(NULL);

        // Get the file name from the dialog box.
        if (!SUCCEEDED(hr))
            return;

        IShellItem* pItem;
        hr = pFileSave->GetResult(&pItem);

        SDL_assert_always(SUCCEEDED(hr));
        PWSTR pszFilePath;
        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

        // Copy path to save_path
        size_t length = wcslen(pszFilePath) * sizeof(*pszFilePath);
        if (save_path) {
            delete[] save_path;
        }
        // @OPTIMIZATION: save_path is probably twice as long as it
        // needs to be?
        save_path = new char[length];

        wcstombs_s(nullptr, save_path, length, pszFilePath, length);

        CoTaskMemFree(pszFilePath);

        SDL_assert_always(SUCCEEDED(hr));

        pItem->Release();
        pFileSave->Release();

        CoUninitialize();
    }

    // Write data
    SDL_RWops* file = SDL_RWFromFile(save_path, "wb");

    SDL_RWwrite(file, &num_splines, sizeof(num_splines), 1);

    for (size_t i = 0; i < num_splines; ++i) {
        SDL_RWwrite(file, &splines[i].points, sizeof(splines[i].points), 1);

        SDL_assert_always(spline_names[i].length() < MAX_SPLINE_NAME_LENGTH);
        SDL_RWwrite(file, spline_names[i].c_str(), sizeof(char),
                    MAX_SPLINE_NAME_LENGTH);
    }

    SDL_RWclose(file);
}

void SplineEditor::load_splines() {
    HRESULT hr =
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    SDL_assert_always(SUCCEEDED(hr));
    IFileOpenDialog* pFileOpen;

    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                          IID_IFileOpenDialog,
                          reinterpret_cast<void**>(&pFileOpen));

    SDL_assert_always(SUCCEEDED(hr));

    COMDLG_FILTERSPEC file_type = {L".spl", L"*.spl"};
    pFileOpen->SetFileTypes(1, &file_type);

    // Show the Open dialog box.
    hr = pFileOpen->Show(NULL);

    if (!SUCCEEDED(hr))
        return;

    // Get the file name from the dialog box.
    IShellItem* pItem;
    hr = pFileOpen->GetResult(&pItem);

    SDL_assert_always(SUCCEEDED(hr));
    PWSTR pszFilePath;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    if (save_path) {
        delete[] save_path;
    }
    size_t length = wcslen(pszFilePath) * sizeof(*pszFilePath);
    // @OPTIMIZATION: save_path is probably twice as long as it
    // needs to be?
    save_path = new char[length];

    wcstombs_s(nullptr, save_path, length, pszFilePath, length);

    CoTaskMemFree(pszFilePath);

    SDL_assert_always(SUCCEEDED(hr));

    pItem->Release();
    pFileOpen->Release();

    CoUninitialize();

    // Read data
    SDL_RWops* file = SDL_RWFromFile(save_path, "rb");

    SDL_RWread(file, &num_splines, sizeof(num_splines), 1);

    spline_names.clear();
    for (size_t i = 0; i < num_splines; ++i) {
        SDL_RWread(file, &splines[i].points, sizeof(splines[i].points), 1);

        char name[MAX_SPLINE_NAME_LENGTH];
        SDL_RWread(file, name, sizeof(char), MAX_SPLINE_NAME_LENGTH);
        spline_names.push_back({name});
    }

    SDL_RWclose(file);

    for (size_t i = 0; i < num_splines; ++i) {
        splines[i].update_render_data();
    }
}

void SplineEditor::init(const Entity* parent_, Spline* splines_,
                        const Bone* limb_bones_[4][2],
                        const char* spline_path) {
    SDL_assert(parent_ != nullptr);
    parent = parent_;
    splines = splines_;

    size_t path_length = strnlen_s(spline_path, MAX_SAVE_PATH_LENGTH) + 1;

    if (save_path)
        delete[] save_path;
    save_path = new char[path_length];

    strcpy_s(save_path, path_length, spline_path);

    for (size_t i = 0; i < 4; ++i) {
        limb_bones[i][0] = limb_bones_[i][0];
        limb_bones[i][1] = limb_bones_[i][1];
    }

    SDL_RWops* file = SDL_RWFromFile(save_path, "rb");

    SDL_RWread(file, &num_splines, sizeof(num_splines), 1);

    char name_buf[32];
    for (size_t i = 0; i < num_splines; ++i) {
        SDL_RWread(file, &splines[i].points, sizeof(splines[i].points), 1);
        splines[i].update_render_data();

        SDL_RWread(file, name_buf, sizeof(char), MAX_SPLINE_NAME_LENGTH);
        spline_names.push_back(name_buf);
    }

    SDL_RWclose(file);
}

void SplineEditor::init(const Entity* parent_, Spline* splines_,
                        size_t num_splines_, const std::string* spline_names_,
                        const Bone* limb_bones_[4][2]) {
    SDL_assert(parent_ != nullptr);
    parent = parent_;
    splines = splines_;
    num_splines = num_splines_;

    for (size_t i = 0; i < 4; ++i) {
        limb_bones[i][0] = limb_bones_[i][0];
        limb_bones[i][1] = limb_bones_[i][1];
    }

    spline_names.reserve(num_splines);
    for (size_t i = 0; i < num_splines; ++i) {
        spline_names.push_back(spline_names_[i]);
    }

    // Init render data for rendering circle
    GLuint circle_indices[CIRCLE_SEGMENTS];
    for (size_t i = 0; i < CIRCLE_SEGMENTS; ++i) {
        circle_indices[i] = static_cast<GLuint>(i);
    }

    circle_vao.init(circle_indices, CIRCLE_SEGMENTS, nullptr, CIRCLE_SEGMENTS);
}

void SplineEditor::update(const MouseKeyboardInput& input) {
    glm::vec2 mouse_pos =
        parent->world_to_local_space(glm::vec3(input.mouse_world_pos(), 0.0f));

    if (creating_new_spline) {
        Spline& selected_spline = splines[selected_spline_index];

        if (!first_point_set && input.mouse_button_down(1)) {
            // Set all points to mouse position and return
            for (auto& p : selected_spline.points) {
                p = mouse_pos;
            }
            selected_spline.update_render_data();
            first_point_set = true;
            return;
        }

        // Set P2 to mouse position and the others to points along the
        // way from P1 to P2 and return
        selected_spline.points[3] = mouse_pos;

        glm::vec2 start_to_end =
            selected_spline.points[3] - selected_spline.points[0];
        selected_spline.points[1] =
            selected_spline.points[0] + start_to_end * 0.3f;
        selected_spline.points[2] =
            selected_spline.points[3] + start_to_end * 0.3f;
        selected_spline.update_render_data();

        if (input.mouse_button_down(1)) {
            creating_new_spline = false;
            first_point_set = false;
        }
        return;
    }

    if (input.mouse_button_up(1)) {
        selected_point_index = static_cast<size_t>(-1);
        return;
    }

    if (input.mouse_button_down(1)) {
        for (size_t n_spline = 0; n_spline < num_splines; ++n_spline) {
            for (size_t n_point = 0; n_point < Spline::num_points; ++n_point) {
                if (glm::length(splines[n_spline].points[n_point] - mouse_pos) <
                    0.2f) {
                    selected_point_index = n_point;
                    selected_spline_index = n_spline;
                }
            }
        }
    }

    if (selected_point_index < 4) {
        auto move_points = [mouse_pos](Spline& spline, size_t point_index) {
            auto& p = spline.points[point_index];
            // If P1 or P2 is selected, also move T1 or T2, respectively
            if (point_index == 0) {
                glm::vec2 move = mouse_pos - p;
                spline.points[1] += move;
            } else if (point_index == 4) {
                glm::vec2 move = mouse_pos - p;
                spline.points[2] += move;
            }

            p = mouse_pos;

            spline.update_render_data();
        };

        move_points(splines[selected_spline_index], selected_point_index);

        if (connect_point_pairs) {
            if (!connect_tangent_pairs &&
                (selected_point_index == 1 || selected_point_index == 2))
                return;

            // Also move partner points/tangents
            size_t partner_spline_index;
            if (selected_spline_index % 2 == 0) {
                partner_spline_index = selected_spline_index + 1;
            } else {
                partner_spline_index = selected_spline_index - 1;
            }
            move_points(splines[partner_spline_index],
                        3 - selected_point_index);
        }
    }
}

void SplineEditor::update_gui() {
    using namespace ImGui;

    Begin("Spline Editor");
    Text("Splines");

    const char** names = new const char*[num_splines];
    for (size_t i = 0; i < num_splines; ++i) {
        names[i] = spline_names[i].c_str();
    }

    int selected = static_cast<int>(selected_spline_index);
    ListBox("", &selected, names, static_cast<int>(num_splines),
            static_cast<int>(num_splines));
    selected_spline_index = static_cast<size_t>(selected);

    delete[] names;

    if (Button("Replace with new spline") && !creating_new_spline) {
        creating_new_spline = true;
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

    if (selected > -1) {
        auto& points = splines[selected_spline_index].points;

        bool value_changed = false;
        const float sensitivity = 0.1f;

        value_changed |= DragFloat2("P1", value_ptr(points[0]), sensitivity,
                                    0.0f, 0.0f, "% .2f");
        value_changed |= DragFloat2("T1", value_ptr(points[1]), sensitivity,
                                    0.0f, 0.0f, "% .2f");
        value_changed |= DragFloat2("T2", value_ptr(points[2]), sensitivity,
                                    0.0f, 0.0f, "% .2f");
        value_changed |= DragFloat2("P2", value_ptr(points[3]), sensitivity,
                                    0.0f, 0.0f, "% .2f");

        if (value_changed) {
            splines[selected_spline_index].update_render_data();
        }
    }

    End();
}

void SplineEditor::render(const Renderer& renderer, bool spline_edit_mode) {
    renderer.debug_shader.use();
    glLineWidth(1.0f);

    static_cast<bool>(spline_edit_mode);

    // Draw circle for selected limb
    renderer.debug_shader.set_color(&Colors::LIGHT_BLUE);
    if (selected_spline_index < num_splines) {
        const Bone** selected_limb_bones =
            limb_bones[selected_spline_index / 2];

        // @OPTIMIZATION: Calculate this stuff less often
        glm::vec4 limb_root_position =
            selected_limb_bones[0]->get_transform() *
            selected_limb_bones[0]->bind_pose_transform[3];
        float radius =
            selected_limb_bones[0]->length + selected_limb_bones[1]->length;

        DebugShader::Vertex circle_vertices[CIRCLE_SEGMENTS];

        for (size_t n_segment = 0; n_segment < CIRCLE_SEGMENTS; ++n_segment) {
            float theta = static_cast<float>(n_segment) /
                          static_cast<float>(CIRCLE_SEGMENTS) * 2.0f * PI;

            circle_vertices[n_segment].pos =
                limb_root_position + glm::vec4(radius * cosf(theta),
                                               radius * sinf(theta), 0.0f,
                                               0.0f);
        }
        circle_vao.update_vertex_data(circle_vertices, CIRCLE_SEGMENTS);
        circle_vao.draw(GL_LINE_LOOP);
    }

    // Draw splines
    renderer.debug_shader.set_color(&Colors::GREEN);

    if (renderer.draw_splines) {
        // Draw all splines (except selected)
        for (size_t i = 0; i < num_splines; ++i) {
            if (i == selected_spline_index && creating_new_spline &&
                !first_point_set)
                continue;

            splines[i].line_vao.draw(GL_LINE_STRIP);
        }
    }

    if (selected_spline_index < num_splines &&
        (!creating_new_spline && !first_point_set)) {
        // Draw selected spline (with points)
        auto& s = splines[selected_spline_index];
        s.line_vao.draw(GL_LINE_STRIP);

        renderer.debug_shader.set_color(&Colors::LIGHT_PURPLE);
        s.point_vao.draw(GL_LINES);

        renderer.debug_shader.set_color(&Colors::PURPLE);
        s.point_vao.draw(GL_POINTS);
    }
}