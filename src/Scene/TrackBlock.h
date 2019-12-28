#pragma once

#include <cstdint>
#include <vector>

#include "Entity.h"

class TrackBlock  {
public:
    TrackBlock(uint32_t blockID, glm::vec3 centerPosition);

    glm::vec3 center;
    uint32_t blockId;
    std::vector<Entity> track;
    std::vector<Entity> objects;
    std::vector<Entity> lanes;
    std::vector<Entity> lights;
    std::vector<Entity> sounds;
};
