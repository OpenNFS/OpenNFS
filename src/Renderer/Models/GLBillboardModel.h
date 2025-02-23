#pragma once

#include "glm/vec3.hpp"
#include "GLModel.h"
/*

const float kBillboardSize = 3.0f;

const std::vector<glm::vec3> verts = {
  glm::vec3(-kBillboardSize, -kBillboardSize, 0), // bottom left corner
  glm::vec3(-kBillboardSize, kBillboardSize, 0),  // top left corner
  glm::vec3(kBillboardSize, kBillboardSize, 0),   // top right corner
  glm::vec3(kBillboardSize, -kBillboardSize, 0),  // bottom right corner
};

class GLBillboardModel : public GLModel {
public:
    GLBillboardModel();

    void update() override{};
    void destroy() override;
    void render() override;
    bool genBuffers() override;

private:
    // OpenGL data
    enum LightVBO : uint8_t { VERTEX = 0, UV, Length };
    GLuint m_lightVertexBuffers[LightVBO::Length]{};
};
*/
