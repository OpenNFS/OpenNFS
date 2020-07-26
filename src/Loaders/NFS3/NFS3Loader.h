#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>

#include "FCE/FceFile.h"
#include "FEDATA/FedataFile.h"
#include "FRD/FrdFile.h"
#include "COL/ColFile.h"
#include "SPEEDS/SpeedsFile.h"
#include "../Shared/CanFile.h"
#include "../Shared/HrzFile.h"
#include "../TrackUtils.h"
#include "../../Config.h"
#include "../../Util/Utils.h"
#include "../../Physics/Car.h"
#include "../../Scene/Track.h"
#include "../../Scene/VirtualRoad.h"
#include "../../Scene/TrackBlock.h"
#include "../../Renderer/Texture.h"

const float NFS3_SCALE_FACTOR = 10.f;

class NFS3Loader
{
public:
    static std::shared_ptr<Car> LoadCar(const std::string &carBasePath);
    static std::shared_ptr<Track> LoadTrack(const std::string &trackBasePath);

private:
    static CarData _ParseFCEModels(const LibOpenNFS::NFS3::FceFile &fceFile);
    static std::vector<OpenNFS::TrackBlock> _ParseTRKModels(const LibOpenNFS::NFS3::FrdFile &frdFile, const std::shared_ptr<Track> &track);
    static std::vector<VirtualRoad> _ParseVirtualRoad(const LibOpenNFS::NFS3::ColFile &colFile);
    static std::vector<Entity> _ParseCOLModels(const LibOpenNFS::NFS3::ColFile &colFile, const std::shared_ptr<Track> &track);
};
