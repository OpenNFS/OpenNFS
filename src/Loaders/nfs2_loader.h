//
// Created by Amrik on 16/01/2018.
//

#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include "trk_loader.h"
#include "../Physics/Car.h"
#include "../Config.h"
#include "../Util/Utils.h"
#include "../Scene/TrackBlock.h"
#include "../nfs_data.h"

#define scaleFactor 100000

using namespace NFS2;

template <typename Platform>  class NFS2_Loader {
public:
    static std::shared_ptr<typename Platform::TRACK> LoadTrack(const std::string &track_base_path);
    static std::shared_ptr<Car> LoadCar(const std::string &track_base_path);

    void dbgPrintVerts(const std::string &path, shared_ptr<typename Platform::TRACK> track);
private:
    // Car
    static std::vector<CarModel> LoadGEO(const std::string &geo_path);
    // Track
    static bool LoadTRK(std::string trk_path, shared_ptr<typename Platform::TRACK> track);
    static bool LoadCOL(std::string col_path, shared_ptr<typename Platform::TRACK> track);
    static void ParseTRKModels(shared_ptr<typename Platform::TRACK> track);
    static std::vector<Track> ParseCOLModels(shared_ptr<typename Platform::TRACK> track);
    static Texture LoadTexture(TEXTURE_BLOCK track_texture, const std::string &track_name, NFSVer nfs_version);
};


