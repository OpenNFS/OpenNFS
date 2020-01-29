#pragma once

#include <glm/vec3.hpp>

class Sound
{
public:
    Sound(glm::vec3 position, uint32_t type);
    Sound() = default;

    glm::vec3 position;
    uint32_t type;
};
