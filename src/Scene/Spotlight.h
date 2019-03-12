//
// Created by amrik on 12/03/19.
//

#pragma once

#include <glm/vec3.hpp>

class Spotlight {
public:
    Spotlight(glm::vec3 position, glm::vec3 direction, glm::vec3 colour, float cutoff);
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 colour;
    float cutOff;
};
