#pragma once

#include <cstdint>
#include <vector>

#include "Entity.h"

namespace OpenNFS {
    class TrackBlock {
    public:
        TrackBlock(uint32_t id, glm::vec3 position, uint32_t virtualRoadStartIndex, uint32_t nVirtualRoadPositions, const std::vector<uint32_t> &neighbourIds);

        uint32_t id;
        glm::vec3 position;
        uint32_t virtualRoadStartIndex;
        uint32_t nVirtualRoadPositions;
        std::vector<uint32_t> neighbourIds;

        std::vector<Entity> track;
        std::vector<Entity> objects;
        std::vector<Entity> lanes;
        std::vector<Entity> lights;
        std::vector<Entity> sounds;
    };
} // namespace OpenNFS
