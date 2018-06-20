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

class NFS3_Loader : trk_loader {
public:
    explicit NFS3_Loader(const std::string &track_base_path);
    TRACK *track = new TRACK();
private:
    bool LoadFRD(std::string frd_path, const std::string &track_name);
    bool LoadCOL(std::string col_path);
    std::vector<TrackBlock> ParseTRKModels();
    std::vector<Track> ParseCOLModels();
    Texture LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name);
};

