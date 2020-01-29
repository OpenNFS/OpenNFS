#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>

#include "FCE/FceFile.h"
#include "FEDATA/FedataFile.h"
#include "FRD/FrdFile.h"
#include "COL/ColFile.h"
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
const uint8_t quadToTriVertNumbers[6] = {0, 1, 2, 0, 2, 3};

class NFS3 {
public:
    static std::shared_ptr<Car> LoadCar(const std::string &carBasePath);
    static std::shared_ptr<Track> LoadTrack(const std::string &trackBasePath);
private:
    static CarData _ParseFCEModels(const FceFile &fceFile);
    static std::vector<TrackBlock> _ParseTRKModels(const FrdFile &frdFile, const std::shared_ptr<Track> &track);
    static std::vector<VirtualRoad> _ParseVirtualRoad(const ColFile &colFile);
    static std::vector<Entity>  _ParseCOLModels(const ColFile &colFile, const std::shared_ptr<Track> &track);
};



