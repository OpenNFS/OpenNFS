//
// Created by Amrik Sadhra on 03/07/2018.
//

#pragma once

#include <sstream>
#include <string>
#include <bitset>
#include <boost/filesystem.hpp>
#include <boost/lambda/bind.hpp>
#include "track_utils.h"
#include "../Physics/Car.h"
#include "../Config.h"
#include "../Util/Utils.h"
#include "../Scene/TrackBlock.h"
#include "../nfs_data.h"

using namespace NFS3_4_DATA;

class NFS4 {
public:
    static std::shared_ptr<Car> LoadCar(const string &car_base_path); // Car
    static std::shared_ptr<TRACK> LoadTrack(const std::string &track_base_path); // Track

private:
    static std::vector<CarModel>  LoadFCE(const std::string &fce_path);
    static bool LoadFRD(const std::string &frd_path, const std::string &track_name, const std::shared_ptr<TRACK> &track);
    static bool LoadCAN(std::string can_path, const std::shared_ptr<TRACK> &track);
    static std::vector<TrackBlock> ParseTRKModels(const std::shared_ptr<TRACK> &track);
    static Texture LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name);
};

