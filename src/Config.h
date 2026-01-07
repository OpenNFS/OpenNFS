#pragma once

#include <fstream>
#include <magic_enum/magic_enum.hpp>
#include <string>
#include <vector>

#include "Common/NFSVersion.h"
#include "Util/Logger.h"

namespace OpenNFS {
    /* --------------- ONFS Compile time parameters here -----------------*/
    /* Some graphics parameters can be found at file SHADER_PREAMBLE_PATH */
    std::string const ONFS_VERSION{"0.4"};
    std::string const ONFS_GL_VERSION{"330"};

    // ----- File paths for asset load/write -----
    std::string const LOG_FILE_PATH{"./"};
    std::string const SHADER_PREAMBLE_PATH{"../shaders/ShaderPreamble.h"};

    std::string const ASSET_PATH{"./assets/"};
    std::string const CAR_PATH{"./assets/car/"};
    std::string const TRACK_PATH{"./assets/tracks/"};
    std::string const RESOURCE_PATH{"../resources/"};

    std::string const NFS_2_TRACK_PATH{"/gamedata/tracks/pc/"};
    std::string const NFS_2_CAR_PATH{"/gamedata/carmodel/pc/"};

    std::string const NFS_2_SE_TRACK_PATH{"/gamedata/tracks/se/"};
    std::string const NFS_2_SE_CAR_PATH{"/gamedata/carmodel/pc/"};

    std::string const NFS_3_TRACK_PATH{"/gamedata/tracks/"};
    std::string const NFS_3_CAR_PATH{"/gamedata/carmodel/"};

    std::string const NFS_4_TRACK_PATH{"/data/tracks/"};
    std::string const NFS_4_CAR_PATH{"/data/cars/"};

    std::string const MCO_TRACK_PATH{"/data/tracks/"};
    std::string const MCO_CAR_PATH{"/data/models/"};

    std::string const NFS_5_TRACK_PATH{"/gamedata/track/"};
    std::string const NFS_5_CAR_PATH{"/gamedata/carmodel/"};

    // ----- File paths for ONFS resources -----
    std::string const RACER_NAME_PATH{"../resources/misc/racers.json"};

    // ----- Graphics -----
    static constexpr uint16_t MAX_TEXTURE_ARRAY_SIZE{512};
    static constexpr uint32_t DEFAULT_X_RESOLUTION{2560};
    static constexpr uint32_t DEFAULT_Y_RESOLUTION{1600};
    static constexpr float DEFAULT_FOV{55.f};
    // Shadow Map Resolution
    static constexpr uint32_t SHADOW_WIDTH{2048}; // Resolution of shadow map
    static constexpr uint32_t SHADOW_HEIGHT{2048};
    // Lighting parameters - These should be adjusted in tandem with ShaderPreamble MAX_CONTRIB limits
    static constexpr uint32_t LIGHTS_PER_NB_BLOCK{3};
    // Number of lights per neighbouring trackblock to contribute to current trackblock lighting
    static constexpr uint32_t NEIGHBOUR_BLOCKS_FOR_LIGHTS{1}; // Number of neighbouring trackblocks to search for lights

    // ----- Defaults -----
    std::string const DEFAULT_CAR{"corv"};
    std::string const DEFAULT_TRACK{"trk000"};
    std::string const DEFAULT_CAR_NFS_VER{magic_enum::enum_name(NFSVersion::NFS_3)};
    std::string const DEFAULT_TRACK_NFS_VER{magic_enum::enum_name(NFSVersion::NFS_3)};
    static constexpr uint32_t DEFAULT_NUM_RACERS{0};

    /* --------------- ONFS Runtime parameters here -----------------*/
    class Config {
      public:
        Config() = default;
        Config(Config const &) = delete;
        Config &operator=(Config const &) = delete;

        static Config &get() {
            static Config instance;
            return instance;
        }

        static void ParseFile(std::ifstream &inStream);
        void InitFromCommandLine(int argc, char **argv);

        // Cmd Line Arg backing variables
        /* -- User Vehicle/Track Params --*/
        std::string car{};
        std::string track{};
        std::string carTag{};
        std::string trackTag{};
        uint32_t nRacers{DEFAULT_NUM_RACERS};
        /* ------- Render Params ------- */
        bool vulkanRender{};
        bool headless{};
        float fov{DEFAULT_FOV};
        uint32_t resX{};
        uint32_t resY{};
        /* -------- Tool Params -------- */
        bool renameAssets{};
        // Easy state pass-around
        uint32_t windowSizeX{};
        uint32_t windowSizeY{};
    };

    struct ParamData {
        float timeScaleFactor{1.f};
        ImVec4 sunAttenuation{0.f, 0.f, 0.f, 1.0f};
        float trackSpecReflectivity{1.f};
        float trackSpecDamper{10.f};
        bool useFrustumCull{true};
        bool physicsDebugView{false};
        bool drawHermiteFrustum{false};
        bool drawTrackAABB{false};
        bool useClassicGraphics{false};
        bool attachCamToHermite{false};
        bool attachCamToCar{true};
        bool drawVroad{false};
        bool drawCAN{false};
        bool drawRaycast{false};
        bool drawAI{true};
        bool simulateCars{true};
        bool drawSkydome{true};
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
} // namespace OpenNFS
