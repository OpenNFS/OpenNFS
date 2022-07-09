#pragma once

#include <glm/vec3.hpp>
#include "Model.h"

class Quad : public Model {
public:
    Quad(glm::vec3 position, glm::vec3 colour, float fromX, float fromY, float toX, float toY);
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
