//
// Created by Amrik on 16/01/2018.
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

using namespace NFS2;

template <typename Platform>  class NFS2_Loader {
public:
    float scaleFactor = 100000;
    explicit NFS2_Loader(const std::string &track_base_path);
    void dbgPrintVerts(const std::string &path);
    typename Platform::TRACK *track = new typename Platform::TRACK();
private:
    bool LoadGEO(std::string geo_path);
    bool LoadPS1GEO(std::string geo_path); // TODO: Clean this up after we know shit works
    bool LoadTRK(std::string trk_path);
    bool LoadCOL(std::string col_path);
    std::vector<TrackBlock> ParseTRKModels();
    std::vector<Track> ParseCOLModels();
    Texture LoadTexture(TEXTURE_BLOCK track_texture, const std::string &track_name, NFSVer nfs_version);
};


