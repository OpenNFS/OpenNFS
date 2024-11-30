#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

#include "Common/NFSVersion.h"
#include "Util/Logger.h"

namespace OpenNFS {
    /* --------------- ONFS Compile time parameters here -----------------*/
    /* Some graphics parameters can be found at file SHADER_PREAMBLE_PATH */
    const std::string ONFS_VERSION = "0.4";
    const std::string ONFS_GL_VERSION = "330";

    // ----- File paths for asset load/write -----
    const std::string LOG_FILE_PATH = "./";
    const std::string SHADER_PREAMBLE_PATH = "../shaders/ShaderPreamble.h";

    const std::string ASSET_PATH = "./assets/";
    const std::string CAR_PATH = ASSET_PATH + "car/";
    const std::string TRACK_PATH = ASSET_PATH + "tracks/";
    const std::string RESOURCE_PATH = "../resources/";

    const std::string NFS_2_TRACK_PATH = "/gamedata/tracks/pc/";
    const std::string NFS_2_CAR_PATH = "/gamedata/carmodel/pc/";

    const std::string NFS_2_SE_TRACK_PATH = "/gamedata/tracks/se/";
    const std::string NFS_2_SE_CAR_PATH = "/gamedata/carmodel/pc/";

    const std::string NFS_3_TRACK_PATH = "/gamedata/tracks/";
    const std::string NFS_3_CAR_PATH = "/gamedata/carmodel/";

    const std::string NFS_4_TRACK_PATH = "/data/tracks/";
    const std::string NFS_4_CAR_PATH = "/data/cars/";

    const std::string MCO_TRACK_PATH = "/data/tracks/";
    const std::string MCO_CAR_PATH = "/data/models/";

    const std::string NFS_5_TRACK_PATH = "/gamedata/track/";
    const std::string NFS_5_CAR_PATH = "/gamedata/carmodel/";

    // ----- Graphics -----
    const uint16_t MAX_TEXTURE_ARRAY_SIZE = 512;
    const uint32_t DEFAULT_X_RESOLUTION = 2560;
    const uint32_t DEFAULT_Y_RESOLUTION = 1600;
    const float DEFAULT_FOV = 55.f;
    // Shadow Map Resolution
    const uint32_t SHADOW_WIDTH = 2048; // Resolution of shadow map
    const uint32_t SHADOW_HEIGHT = 2048;
    // Lighting parameters - These should be adjusted in tandem with ShaderPreamble MAX_CONTRIB limits
    const uint32_t LIGHTS_PER_NB_BLOCK = 3;
    // Number of lights per neighbouring trackblock to contribute to current trackblock lighting
    const uint32_t NEIGHBOUR_BLOCKS_FOR_LIGHTS = 1; // Number of neighbouring trackblocks to search for lights

    // ----- Defaults -----
    const std::string DEFAULT_CAR = "corv";
    const std::string DEFAULT_TRACK = "trk000";
    const std::string DEFAULT_CAR_NFS_VER = get_string(NFSVersion::NFS_3);
    const std::string DEFAULT_TRACK_NFS_VER = get_string(NFSVersion::NFS_3);
    const uint32_t DEFAULT_NUM_RACERS = 0;

    /* --------------- ONFS Runtime parameters here -----------------*/
    class Config {
    public:
        static Config &get() {
            static Config instance;
            return instance;
        }

        void ParseFile(std::ifstream &inStream);

        void InitFromCommandLine(int argc, char **argv);

        // Cmd Line Arg backing variables
        /* -- User Vehicle/Track Params --*/
        std::string car;
        std::string track;
        std::string carTag;
        std::string trackTag;
        uint32_t nRacers = DEFAULT_NUM_RACERS;
        /* ----- Physics/AI Params ----- */
        bool useFullVroad{true};
        /* ------- Render Params ------- */
        bool vulkanRender{};
        bool headless{};
        float fov = DEFAULT_FOV;
        uint32_t resX{};
        uint32_t resY{};
        /* -- Tool Params -- */
        bool renameAssets{};
        // Easy state pass-around
        uint32_t windowSizeX{};
        uint32_t windowSizeY{};

    private:
        Config() = default;

        Config(const Config &);

        Config &operator=(const Config &);
    };

    struct ParamData {
        float timeScaleFactor = 1.f;
        ImVec4 sunAttenuation = ImVec4(0.f, 0.f, 0.f, 1.0f);
        float trackSpecReflectivity = 1;
        float nearPlane = 160.f;
        float farPlane = 300.f;
        float trackSpecDamper = 10;
        int blockDrawDistance = 15;
        bool useFrustumCull = true;
        bool physicsDebugView = false;
        bool drawHermiteFrustum = false;
        bool drawTrackAABB = false;
        bool useClassicGraphics = false;
        bool attachCamToHermite = false;
        bool attachCamToCar = true;
        bool drawVroad = false;
        bool drawCAN = false;
        bool drawRaycast = false;
        bool simulateCars = true;
        bool drawSkydome = true;
    };

    struct AssetData {
        NFSVersion carTag;
        std::string car;
        NFSVersion trackTag;
        std::string track;
    };

    struct NfsAssetList {
        NFSVersion tag;
        std::vector<std::string> tracks;
        std::vector<std::string> cars;
    };
}
