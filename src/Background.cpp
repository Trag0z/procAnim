#pragma once
#include "pch.h"
#include "Background.h"
#include "Renderer.h"

void Background::init(const char* texture_path) {
    Entity::init();
    tex.load_from_file(texture_path);

    GLuint indices[6] = {0, 1, 2, 2, 3, 0};

    TexturedShader::Vertex vertices[4];
    vertices[0] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
    vertices[1] = {{0.0f, tex.dimensions.y}, {0.0f, 1.0f}};
    vertices[2] = {{tex.dimensions.x, tex.dimensions.y}, {1.0f, 1.0f}};
    vertices[3] = {{tex.dimensions.x, 0.0f}, {1.0f, 0.0f}};

    vao.init(indices, 6, vertices, 4);
}

void Background::render(const Renderer& renderer, glm::vec2 camera_position) {
    renderer.textured_shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    // Render the background 9 times, for the quadrant that the camera is in and
    // all the ones around it
    glm::vec2 cam_quadrant = {
        std::floorf(camera_position.x / tex.dimensions.x),
        std::floorf(camera_position.y / tex.dimensions.y)};

    glm::mat3 model_mat;

    for (float x = -1.0f; x < 2.0f; x += 1.0f) {
        for (float y = -1.0f; y < 2.0f; y += 1.0f) {
            model_mat = glm::translate(model, (cam_quadrant - glm::vec2(x, y)) *
                                                  tex.dimensions);
            renderer.textured_shader.set_model(&model_mat);
            vao.draw(GL_TRIANGLES);
        }
    }
}