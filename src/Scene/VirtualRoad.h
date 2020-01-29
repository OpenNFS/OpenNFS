#pragma once

#include <glm/glm.hpp>

class VirtualRoad
{
public:
    VirtualRoad(glm::vec3 position, uint32_t unknown, glm::vec3 normal, glm::vec3 forward, glm::vec3 right, glm::vec3 leftWall, glm::vec3 rightWall);
    VirtualRoad() = default;
    glm::vec3 position;
    uint32_t unknown;  // Unknown data
    glm::vec3 normal, forward, right;
    glm::vec3 leftWall, rightWall;
};
