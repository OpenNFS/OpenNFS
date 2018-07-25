//
// Created by Amrik on 16/01/2018.
//

#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include "track_utils.h"
#include "../Physics/Car.h"
#include "../Config.h"
#include "../Util/Utils.h"
#include "../Scene/TrackBlock.h"
#include "../nfs_data.h"

#define scaleFactor 100000

using namespace NFS2_DATA;

void DumpToObj(int block_Idx, PS1::GEO::BLOCK_HEADER *geoBlockHeader, PS1::GEO::BLOCK_3D *vertices, PS1::GEO::BLOCK_3D *normals, PS1::GEO::POLY_3D *polygons);

template <typename Platform>  class NFS2 {
public:
    static std::shared_ptr<typename Platform::TRACK> LoadTrack(const std::string &track_base_path);
    static std::shared_ptr<Car> LoadCar(const std::string &track_base_path);

    static void dbgPrintVerts(const std::string &path, shared_ptr<typename Platform::TRACK> track);
    static bool ExtractPSH(const std::string &psh_path, const std::string &output_path);
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


