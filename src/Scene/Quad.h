//
// Created by Amrik Sadhra on 01/03/2018.
//

#pragma once

#include "Model.h"
#include <glm/vec3.hpp>

class Quad : public Model {
  public:
    Quad(glm::vec3 position, glm::vec3 colour, float size);
    Quad();

    void update() override;
    void destroy() override;
    void render() override;
    bool genBuffers() override;

    glm::vec3 colour;
    long type;

  private:
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalBuffer;
    typedef Model super;
};
