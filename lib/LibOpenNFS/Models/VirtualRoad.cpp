#include "VirtualRoad.h"

namespace LibOpenNFS {
    VirtualRoad::VirtualRoad(glm::vec3 position, glm::vec3 respawn, glm::vec3 normal, glm::vec3 forward, glm::vec3 right, glm::vec3 leftWall, glm::vec3 rightWall,
                             uint32_t unknown) {
        this->position  = position;
        this->respawn   = respawn;
        this->normal    = normal;
        this->forward   = forward;
        this->right     = right;
        this->leftWall  = leftWall;
        this->rightWall = rightWall;
        this->unknown   = unknown;
    }
} // namespace LibOpenNFS
