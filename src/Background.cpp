#pragma once

#include "Background.h"
#include "rendering/Renderer.h"

void Background::init(const char* texture_path) {
    Entity::init();
    texture.load_from_file(texture_path);

    GLuint indices[6] = {0, 1, 2, 2, 3, 0};

    TexturedShader::Vertex vertices[4];
    vertices[0] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
    vertices[1] = {{0.0f, texture.dimensions.y}, {0.0f, 1.0f}};
    vertices[2] = {{texture.dimensions.x, texture.dimensions.y}, {1.0f, 1.0f}};
    vertices[3] = {{texture.dimensions.x, 0.0f}, {1.0f, 0.0f}};

    vao.init(indices, 6, vertices, 4, GL_STATIC_DRAW);
}

void Background::render(const Renderer& renderer, glm::vec2 camera_position) {
    renderer.textured_shader.use();
    renderer.textured_shader.set_texture(texture);

    // Render the background 9 times, for the quadrant that the camera is in and
    // all the ones around it
    glm::vec2 cam_quadrant = {
        std::floorf(camera_position.x / texture.dimensions.x),
        std::floorf(camera_position.y / texture.dimensions.y)};

    glm::mat3 model_mat;

    for (float x = -1.0f; x < 2.0f; x += 1.0f) {
        for (float y = -1.0f; y < 2.0f; y += 1.0f) {
            model_mat = glm::translate(model, (cam_quadrant - glm::vec2(x, y)) *
                                                  texture.dimensions);
            renderer.textured_shader.set_model(&model_mat);
            vao.draw(GL_TRIANGLES);
        }
    }
}