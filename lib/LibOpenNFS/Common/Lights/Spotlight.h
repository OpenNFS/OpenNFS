#pragma once

#include "glm/vec3.hpp"

class Spotlight {
public:
    Spotlight(glm::vec3 position, glm::vec3 direction, glm::vec3 colour, float cutoff);
    Spotlight() = default;
    glm::vec3 position;
    glm::vec3 initialPosition;
    glm::vec3 direction;
    glm::vec3 colour;
    float cutOff;
};
