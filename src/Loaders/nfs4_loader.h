//
// Created by Amrik on 03/07/2018.
//

#pragma once

#include <sstream>
#include <string>
#include <boost/filesystem/path.hpp>
#include "../Config.h"
#include "../nfs_data.h"
#include "../Util/Utils.h"

using namespace Utils;
using namespace NFS3_4;

class NFS4_Loader {
public:
    explicit NFS4_Loader(const std::string &track_base_path); // Track
    TRACK *track = new TRACK();

    NFS4_Loader(const string &car_base_path, string *car_name); // Car
    std::vector<CarModel> meshes;
private:
    bool LoadFCE(const std::string &fce_path);
    bool LoadFRD(const std::string &frd_path, const std::string &track_name);
    std::vector<TrackBlock> ParseTRKModels();
    Texture LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name);
};

