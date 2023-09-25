#pragma once

#include <cstdint>
#include <vector>

#include "TrackEntity.h"

namespace LibOpenNFS {
    class TrackBlock {
    public:
        TrackBlock(uint32_t id, glm::vec3 position, uint32_t virtualRoadStartIndex, uint32_t nVirtualRoadPositions, const std::vector<uint32_t> &neighbourIds);

        uint32_t id;
        glm::vec3 position;
        uint32_t virtualRoadStartIndex;
        uint32_t nVirtualRoadPositions;
        std::vector<uint32_t> neighbourIds;

        std::vector<TrackEntity> track;
        std::vector<TrackEntity> objects;
        std::vector<TrackEntity> lanes;
        std::vector<TrackEntity> lights;
        std::vector<TrackEntity> sounds;
    };
} // namespace LibOpenNFS
