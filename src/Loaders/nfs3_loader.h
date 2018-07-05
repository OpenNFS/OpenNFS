//
// Created by Amrik.Sadhra on 20/06/2018.
//

#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include "trk_loader.h"
#include "../Config.h"
#include "../Util/Utils.h"
#include "../Scene/TrackBlock.h"
#include "../nfs_data.h"

using namespace NFS3;
using namespace Utils;

void ConvertFCE(const std::string &fce_path, const std::string &obj_out_path);

class NFS3_Loader : trk_loader {
public:
    explicit NFS3_Loader(const std::string &track_base_path); // Track
    TRACK *track = new TRACK();

    NFS3_Loader(const string &car_base_path, string *car_name); // Car
    std::vector<CarModel> meshes;
    void WriteObj(const std::string &path);
private:
    // Car
    bool LoadFCE(const std::string fce_path);

    // Track
    bool LoadFRD(std::string frd_path, const std::string &track_name);
    bool LoadCOL(std::string col_path);
    std::vector<TrackBlock> ParseTRKModels();
    std::vector<Track> ParseCOLModels();
    Texture LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name);
};

