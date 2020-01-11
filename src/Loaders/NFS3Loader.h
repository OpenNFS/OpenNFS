#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include "TrackUtils.h"
#include "NFS3/FrdFile.h"
#include "../Physics/Car.h"
#include "../Config.h"
#include "../Util/ImageLoader.h"
#include "../Util/Utils.h"
#include "../Scene/TrackBlock.h"
#include "../Scene/Lights/LightModel.h"
#include "../nfs_data.h"

using namespace NFS3_4_DATA;

const float NFS3_SCALE_FACTOR = 10.f;

class NFS3 {
public:
    static std::shared_ptr<Car> LoadCar(const std::string &car_base_path);
    static std::shared_ptr<TRACK> LoadTrack(const std::string &track_base_path);
    static bool LoadFFN(const std::string &ffn_path);

    static void ConvertFCE(const std::string &fce_path, const std::string &obj_out_path);
    // Car (Expose for GA training)
    static CarData LoadFCE(const std::string &fce_path);
private:
    // Track
    static bool LoadCOL(std::string col_path, const std::shared_ptr<TRACK> &track);
    static bool LoadHRZ(std::string hrz_path, const std::shared_ptr<TRACK> &track);
    static std::vector<TrackBlock> ParseTRKModels(const FrdFile &frdFile, const std::shared_ptr<TRACK> &track);
    static std::vector<Entity>  ParseCOLModels(const std::shared_ptr<TRACK> &track);
    static Texture LoadTexture(TexBlock trackTexture, const std::string &trackName);
};



