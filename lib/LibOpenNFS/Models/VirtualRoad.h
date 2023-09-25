#pragma once

#include "lib/glm/glm/glm.hpp"

namespace LibOpenNFS {
    class VirtualRoad {
    public:
        VirtualRoad(glm::vec3 position, glm::vec3 respawn, glm::vec3 normal, glm::vec3 forward, glm::vec3 right, glm::vec3 leftWall, glm::vec3 rightWall, uint32_t unknown);
        VirtualRoad() = default;
        glm::vec3 position;
        glm::vec3 normal, forward, right;
        glm::vec3 leftWall, rightWall, respawn;
        uint32_t unknown; // Unknown data?
    };
} // namespace LibOpenNFS
