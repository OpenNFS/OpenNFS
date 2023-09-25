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
#include "../../../src/Physics/Car.h"
#include "Models/Track.h"
#include "Models/TrackBlock.h"
#include "Models/VirtualRoad.h"
#include "Models/Texture.h"

namespace LibOpenNFS::NFS3 {
    const float NFS3_SCALE_FACTOR = 10.f;

    class Loader {
    public:
        static std::shared_ptr<Car> LoadCar(const std::string &carBasePath);
        static std::shared_ptr<Track> LoadTrack(const std::string &trackBasePath);

    private:
        static CarData _ParseFCEModels(const LibOpenNFS::NFS3::FceFile &fceFile);
        static std::vector<TrackBlock> _ParseTRKModels(const LibOpenNFS::NFS3::FrdFile &frdFile, const std::shared_ptr<Track> &track);
        static std::vector<VirtualRoad> _ParseVirtualRoad(const LibOpenNFS::NFS3::ColFile &colFile);
        static std::vector<TrackEntity> _ParseCOLModels(const LibOpenNFS::NFS3::ColFile &colFile, const std::shared_ptr<Track> &track);
    };
} // namespace LibOpenNFS::NFS3
