#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>

#include "TrackEntity.h"
#include "Models/TrackBlock.h"
#include "Models/VirtualRoad.h"

#include <Shared/CanFile.h>
#include "Texture.h"

namespace LibOpenNFS {
    class Track {
public:
    Track() = default;
    ~Track() = default;

    // Metadata
    NFSVersion nfsVersion{};
    std::string name;
    uint32_t nBlocks{0};
    std::vector<CameraAnimPoint> cameraAnimation;
    std::vector<VirtualRoad> virtualRoad;

    // Geometry
    std::vector<TrackBlock> trackBlocks;
    std::vector<TrackEntity> globalObjects;
    std::vector<TrackEntity> vroadBarriers;

    // GL 3D Render Data
    std::map<uint32_t, Texture> textureMap;
};
}
