#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
//
// Created by Amrik.Sadhra on 13/06/2018.
//

#pragma once

#include <imgui.h>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <boost/program_options.hpp>

#include "Enums.h"
#include "Util/Logger.h"

using namespace boost::program_options;

const std::string ONFS_VERSION = "0.3";
const std::string ONFS_GL_VERSION = "330";

const std::string LOG_FILE_PATH = "./";

const std::string ASSET_PATH= "./assets/";
const std::string CAR_PATH = ASSET_PATH + "car/";
const std::string TRACK_PATH = ASSET_PATH + "tracks/";
const std::string RESOURCE_PATH = "../resources/";

const std::string BEST_NETWORK_PATH = ASSET_PATH + "bestRacer.net";

const std::string NFS_2_TRACK_PATH = "/GAMEDATA/TRACKS/PC/";
const std::string NFS_2_CAR_PATH = "/GAMEDATA/CARMODEL/PC/";

const std::string NFS_2_SE_TRACK_PATH = "/GAMEDATA/TRACKS/SE/";
const std::string NFS_2_SE_CAR_PATH = "/GAMEDATA/CARMODEL/PC/";

const std::string NFS_3_TRACK_PATH = "/gamedata/tracks/";
const std::string NFS_3_CAR_PATH  = "/gamedata/carmodel/";

const std::string NFS_4_TRACK_PATH = "/DATA/TRACKS/";
const std::string NFS_4_CAR_PATH = "/DATA/CARS/";

const uint16_t MAX_TEXTURE_ARRAY_SIZE = 512;

const uint32_t DEFAULT_X_RESOLUTION = 1920;
const uint32_t DEFAULT_Y_RESOLUTION = 1080;

const std::string DEFAULT_CAR = "diab";
const std::string DEFAULT_TRACK = "trk001";

class Config
{
public:
    static Config& get()
    {
        static Config instance;
        return instance;
    }
    void ParseFile(std::ifstream& inStream);
    void InitFromCommandLine(int argc, char **argv);
    template<typename _T>
    _T getValue(const std::string &key){
        return storedConfig[key].as<_T>();
    };
    // Better named parameters instead of using var_map with command-line arg name
    std::string car = DEFAULT_CAR, track = DEFAULT_TRACK;
    /* -- Render Params -- */
    bool vulkanRender = false;
    uint32_t resX = DEFAULT_X_RESOLUTION, resY = DEFAULT_Y_RESOLUTION;
    /* -- Training Params -- */
    bool trainingMode = false;
    uint16_t populationSize, nGenerations;
    uint32_t nTicks;
private:
    Config() = default;
    Config(const Config&);
    Config& operator=(const Config&);
    variables_map storedConfig;
};

struct ParamData {
    float timeScaleFactor = 1.f;
    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.0f);
    ImVec4 sun_attenuation = ImVec4(0.f, 0.f, 0.f, 1.0f);
    ImVec4 nfs2_rotate = ImVec4(0.f, 0.f, 0.f, 0.f);
    float trackSpecReflectivity = 1;

    float trackSpecDamper = 10;
    int blockDrawDistance = 15;
    bool window_active = true;
    bool physics_debug_view = false;
    bool use_classic_graphics = false;
    bool attach_cam_to_hermite = false;
    bool use_nb_data = false;
    bool attach_cam_to_car = true;
    bool frustum_cull = false;
    bool draw_vroad = false;
    bool draw_can = true;

    bool draw_raycast = true;
    bool simulate_car = false;
};

struct AssetData {
    NFSVer carTag;
    std::string car;
    NFSVer trackTag;
    std::string track;
};

struct NeedForSpeed {
    NFSVer tag;
    std::vector<std::string> tracks;
    std::vector<std::string> cars;
};
#pragma clang diagnostic pop