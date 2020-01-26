#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>

#include "FCE/FceFile.h"
#include "FEDATA/FedataFile.h"
#include "FRD/FrdFile.h"
#include "COL/ColFile.h"

#include "../TrackUtils.h"
#include "../../Physics/Car.h"
#include "../../Config.h"
#include "../../Util/Utils.h"
#include "../../Scene/TrackBlock.h"

const float NFS3_SCALE_FACTOR = 10.f;
const uint8_t quadToTriVertNumbers[6] = {0, 1, 2, 0, 2, 3};

class NFS3 {
public:
    static std::shared_ptr<Car> LoadCar(const std::string &carBasePath);
    static std::shared_ptr<TRACK> LoadTrack(const std::string &trackBasePath);
private:
    static CarData _ParseFCEModels(const FceFile &fceFile);
    static std::vector<TrackBlock> _ParseTRKModels(const FrdFile &frdFile);
    static std::vector<Entity>  _ParseCOLModels(const ColFile &colFile);
};



