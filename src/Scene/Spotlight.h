#pragma once

#include "glm/vec3.hpp"

#include <Entities/BaseLight.h>

namespace OpenNFS {
    class Spotlight final : public LibOpenNFS::BaseLight {
      public:
        Spotlight();
        Spotlight(glm::vec3 position, glm::vec3 direction, glm::vec3 colour, float cutoff);
        glm::vec3 initialPosition;
        glm::vec3 direction;
        float cutOff;
    };
} // namespace OpenNFS
