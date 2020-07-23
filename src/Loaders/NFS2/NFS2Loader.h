#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <bitset>

#include "bmpread.h"
#include "TRK/TrkFile.h"
#include "COL/ColFile.h"
#include "../TrackUtils.h"
#include "../../Config.h"
#include "../../Util/Utils.h"
#include "../../Physics/Car.h"
#include "../../Scene/Track.h"
#include "../../Scene/VirtualRoad.h"
#include "../../Scene/TrackBlock.h"

const float NFS2_SCALE_FACTOR = 1000000.0f;

template <typename Platform>
class NFS2Loader
{
public:
    static std::shared_ptr<Car> LoadCar(const std::string &carBasePath);
    static std::shared_ptr<Track> LoadTrack(const std::string &trackBasePath);

private:
    // static CarData _ParseGEOModels(const GeoFile &fceFile);
    static std::vector<OpenNFS::TrackBlock> _ParseTRKModels(const LibOpenNFS::NFS2::TrkFile<Platform> &trkFile, const std::shared_ptr<Track> &track);
    static std::vector<VirtualRoad> _ParseVirtualRoad(const LibOpenNFS::NFS2::ColFile<Platform> &colFile);
    static std::vector<Entity> _ParseCOLModels(const LibOpenNFS::NFS2::ColFile<Platform> &colFile, const std::shared_ptr<Track> &track);
    // static Texture LoadTexture(TEXTURE_BLOCK track_texture, const std::string &track_name, NFSVer nfs_version);
};
