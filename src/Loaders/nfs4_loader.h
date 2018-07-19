//
// Created by Amrik on 03/07/2018.
//

#pragma once

#include <sstream>
#include <string>
#include <boost/filesystem/path.hpp>
#include "trk_loader.h"
#include "../Physics/Car.h"
#include "../Config.h"
#include "../Util/Utils.h"
#include "../Scene/TrackBlock.h"
#include "../nfs_data.h"

using namespace Utils;
using namespace NFS3_4;

class NFS4 {
public:
    static std::shared_ptr<Car> LoadCar(const string &car_base_path, string *car_name); // Car
    static std::shared_ptr<TRACK> LoadTrack(const std::string &track_base_path); // Track

private:
    static std::vector<CarModel>  LoadFCE(const std::string &fce_path);
    static bool LoadFRD(const std::string &frd_path, const std::string &track_name, std::shared_ptr<TRACK> track);
    static std::vector<TrackBlock> ParseTRKModels(std::shared_ptr<TRACK> track);
    static Texture LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name);
};

