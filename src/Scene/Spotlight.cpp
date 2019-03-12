//
// Created by amrik on 12/03/19.
//

#include "Spotlight.h"

Spotlight::Spotlight(glm::vec3 position, glm::vec3 direction, glm::vec3 colour, float cutOff) {
    this->position = position;
    this->direction = direction;
    this->colour = colour;
    this->cutOff = cutOff;
}
