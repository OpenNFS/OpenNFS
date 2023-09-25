#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <bitset>

#include "bmpread.h"
#include "GEO/GeoFile.h"
#include "TRK/TrkFile.h"
#include "COL/ColFile.h"
#include "../../../src/Physics/Car.h"
#include "Models/Track.h"
#include "Models/VirtualRoad.h"
#include "Models/TrackBlock.h"

namespace LibOpenNFS::NFS2 {
    const float NFS2_SCALE_FACTOR = 1000000.0f;

    template <typename Platform>
    class Loader {
    public:
        static std::shared_ptr<Car> LoadCar(const std::string &carBasePath, NFSVersion nfsVersion);
        static std::shared_ptr<Track> LoadTrack(const std::string &trackBasePath, NFSVersion nfsVersion);

    private:
        static CarData _ParseGEOModels(const LibOpenNFS::NFS2::GeoFile<Platform> &geoFile);
        static std::vector<LibOpenNFS::TrackBlock> _ParseTRKModels(const LibOpenNFS::NFS2::TrkFile<Platform> &trkFile, LibOpenNFS::NFS2::ColFile<Platform> &colFile,
                                                                const std::shared_ptr<Track> &track);
        static std::vector<VirtualRoad> _ParseVirtualRoad(LibOpenNFS::NFS2::ColFile<Platform> &colFile);
        static std::vector<TrackEntity> _ParseCOLModels(LibOpenNFS::NFS2::ColFile<Platform> &colFile, const std::shared_ptr<Track> &track);
    };
} // namespace LibOpenNFS::NFS2
