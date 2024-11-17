#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>

#include "Common/TextureUtils.h"
#include "FCE/FceFile.h"
#include "FEDATA/FedataFile.h"
#include "FRD/FrdFile.h"
#include "COL/ColFile.h"
#include "SPEEDS/SpeedsFile.h"
#include "Shared/CanFile.h"
#include "Shared/HrzFile.h"
#include "Shared/VivFile.h"
#include "Entities/Car.h"
#include "Entities/Track.h"
#include "Entities/TrackBlock.h"
#include "Entities/TrackVRoad.h"
#include "Entities/TrackTextureAsset.h"

namespace LibOpenNFS::NFS3 {
    const glm::vec3 NFS3_SCALE_FACTOR(-0.1, 0.1, 0.1f);

    class Loader {
    public:
        static Car LoadCar(const std::string &carBasePath);
        static Track LoadTrack(const std::string &trackBasePath);

    private:
        static Car::MetaData _ParseAssetData(const FceFile &fceFile, const FedataFile &fedataFile);
        static std::map<uint32_t, TrackTextureAsset> _ParseTextures(const FrdFile &frdFile, const Track &track);
        static std::vector<TrackBlock> _ParseTRKModels(const FrdFile &frdFile, const Track &track);
        static std::vector<TrackVRoad> _ParseVirtualRoad(const ColFile &colFile);
        static std::vector<TrackEntity> _ParseCOLModels(const ColFile &colFile, const Track &track, std::vector<TexBlock> &texBlocks);
    };
} // namespace LibOpenNFS::NFS3
