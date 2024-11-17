#pragma once

#include <cstdint>
#include <vector>
#include <map>

#include "TrackEntity.h"
#include "TrackBlock.h"
#include "TrackVRoad.h"

#include <Shared/CanFile.h>
#include "TrackTextureAsset.h"

namespace LibOpenNFS {
    class Track {
public:
    Track(NFSVersion _nfsVersion, std::string const& _name, std::string const& _basePath);
    Track() = default;

    // Metadata
    NFSVersion nfsVersion{};
    std::string name;
    std::string basePath;
    uint32_t nBlocks{0};
    std::vector<CameraAnimPoint> cameraAnimation;
    std::vector<TrackVRoad> virtualRoad;

    // Geometry
    std::map<uint32_t, TrackTextureAsset> trackTextureAssets;
    std::vector<TrackBlock> trackBlocks;
    std::vector<TrackEntity> globalObjects;
    std::vector<TrackEntity> vroadBarriers;
};
}
