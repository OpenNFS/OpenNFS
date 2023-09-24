#pragma once

#include "glm/vec3.hpp"
#include "../../../../src/Scene/Models/Model.h"

const float kLightSize = 3.0f;

const std::vector<glm::vec3> verts = {
  glm::vec3(-kLightSize, -kLightSize, 0), // bottom left corner
  glm::vec3(-kLightSize, kLightSize, 0),  // top left corner
  glm::vec3(kLightSize, kLightSize, 0),   // top right corner
  glm::vec3(kLightSize, -kLightSize, 0),  // bottom right corner
};

class LightModel : public Model {
public:
    LightModel();

    void update() override{};
    void destroy() override;
    void render() override;
    bool genBuffers() override;

private:
    // OpenGL data
    enum LightVBO : uint8_t { VERTEX = 0, UV, Length };
    GLuint m_lightVertexBuffers[LightVBO::Length]{};
};
