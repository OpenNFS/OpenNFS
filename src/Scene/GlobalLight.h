#pragma once

#include <glm/glm.hpp>

class GlobalLight
{
public:
    explicit GlobalLight();
    glm::mat4 lightSpaceMatrix;
    glm::mat4 viewMatrix;
    glm::vec3 position;
};
