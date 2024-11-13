#pragma once

#include "TrackEntity.h"
#include "lib/glm/glm/vec3.hpp"

namespace LibOpenNFS {
    class TrackSound : TrackEntity {
    public:
        TrackSound(uint32_t entityID, glm::vec3 position, uint32_t type);

        glm::vec3 position;
        uint32_t type;
    };
} // namespace LibOpenNFS