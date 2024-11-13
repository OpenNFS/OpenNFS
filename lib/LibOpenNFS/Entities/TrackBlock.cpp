#include "TrackBlock.h"

namespace LibOpenNFS {
    TrackBlock::TrackBlock(uint32_t id, glm::vec3 position, uint32_t virtualRoadStartIndex, uint32_t nVirtualRoadPositions, const std::vector<uint32_t> &neighbourIds) {
        this->id                    = id;
        this->position              = position;
        this->virtualRoadStartIndex = virtualRoadStartIndex;
        this->nVirtualRoadPositions = nVirtualRoadPositions;
        this->neighbourIds          = neighbourIds;
    }
} // namespace LibOpenNFS
