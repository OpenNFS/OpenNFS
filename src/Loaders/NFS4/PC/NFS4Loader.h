#pragma once

#include <sstream>
#include <string>
#include <bitset>
#include <boost/filesystem.hpp>
#include <boost/lambda/bind.hpp>
#include "../../TrackUtils.h"
#include "../../../Physics/Car.h"
#include "../../../Config.h"
#include "../../../Util/ImageLoader.h"
#include "../../../Util/Utils.h"
#include "../../../Scene/TrackBlock.h"
#include "../../../nfs_data.h"

using namespace NFS3_4_DATA;

class NFS4 {
public:
    static std::shared_ptr<Car> LoadCar(const std::string &car_base_path, NFSVer version); // Car
    static std::shared_ptr<TRACK> LoadTrack(const std::string &track_base_path); // TrackModel

private:
    static CarData LoadFCE(const std::string &fce_path, NFSVer version);
    static bool LoadFRD(const std::string &frd_path, const std::string &track_name, const std::shared_ptr<TRACK> &track);
    static std::vector<TrackBlock> ParseTRKModels(const std::shared_ptr<TRACK> &track);
    static Texture LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name);
};

