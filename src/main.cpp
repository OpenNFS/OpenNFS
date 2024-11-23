#ifdef VULKAN_BUILD
#define GLFW_INCLUDE_VULKAN
#include "Renderer/vkRenderer.h"
#endif

#include <cstdlib>
#include <filesystem>
#include <string>
#include <iostream>
#include <LibOpenNFS.h>

#include "Config.h"
#include "Util/Logger.h"
#include "Scene/Track.h"
#include "Loaders/TrackLoader.h"
#include "Loaders/CarLoader.h"
#include "Loaders/MusicLoader.h"
#include "Renderer/Renderer.h"
#include "Race/RaceSession.h"

using namespace std::filesystem;
using namespace OpenNFS;

class OpenNFSEngine {
public:
    explicit OpenNFSEngine(const std::shared_ptr<Logger> &onfs_logger) : logger(onfs_logger) {
        if (Config::get().renameAssets) {
            OpenNFS::Utils::RenameAssetsToLowercase();
        }
        InitDirectories();
        installedNFS = OpenNFS::Utils::PopulateAssets();

        if (Config::get().vulkanRender) {
#ifdef VULKAN_BUILD
            vkRenderer renderer;
            renderer.run();
#else
            CHECK_F(false, "This build of OpenNFS was not compiled with Vulkan support!");
#endif
        } else {
            run();
        }
    }

    void run() const {
        LOG(INFO) << "OpenNFS Version " << ONFS_VERSION;

        // Must initialise OpenGL here as the Loaders instantiate meshes which create VAO's
        std::shared_ptr<GLFWwindow> const window{Renderer::InitOpenGL(Config::get().resX, Config::get().resY, "OpenNFS v" + ONFS_VERSION)};
        AssetData loadedAssets{
            get_enum(Config::get().carTag), Config::get().car, get_enum(Config::get().trackTag), Config::get().track
        };

        const std::function logInfoCallback = [](const std::string &logMessage) {
            LogCapture(__FILE__, __LINE__, __FUNCTION__, INFO).stream() << logMessage;
        };
        const std::function logWarnCallback = [](const std::string &logMessage) {
            LogCapture(__FILE__, __LINE__, __FUNCTION__, WARNING).stream() << logMessage;
        };
        const std::function logDebugCallback = [](const std::string &logMessage) {
            LogCapture(__FILE__, __LINE__, __FUNCTION__, DEBUG).stream() << logMessage;
        };

        LibOpenNFS::RegisterLogCallback(LONFS_INFO, logInfoCallback);
        LibOpenNFS::RegisterLogCallback(LONFS_WARNING, logWarnCallback);
        LibOpenNFS::RegisterLogCallback(LONFS_DEBUG, logDebugCallback);

        // TODO: TEMP FIX UNTIL I DO A PROPER RETURN from race session
        CHECK_F(loadedAssets.trackTag != NFSVersion::UNKNOWN, "Unknown track type!");

        /*------- Render --------*/
        while (loadedAssets.trackTag != NFSVersion::UNKNOWN) {
            /*------ ASSET LOAD ------*/
            // Load Track Data
            auto const &track = TrackLoader::Load(loadedAssets.trackTag, loadedAssets.track);
            // Load Car data from unpacked NFS files (TODO: Track first (for now), silly dependence on extracted sky texture for car environment map)
            auto const &car = CarLoader::LoadCar(loadedAssets.carTag, loadedAssets.car);

            // Load Music
            // MusicLoader musicLoader("F:\\NFS3\\nfs3_modern_base_eng\\gamedata\\audio\\pc\\atlatech");

            RaceSession race(window, logger, installedNFS, track, car);
            loadedAssets = race.Simulate();
        }

        // Close OpenGL window and terminate GLFW
        glfwTerminate();
    }

private:
    std::shared_ptr<Logger> logger;
    std::vector<NfsAssetList> installedNFS;

    static void InitDirectories() {
        if (!(exists(OpenNFS::CAR_PATH))) {
            create_directories(OpenNFS::CAR_PATH);
        }
        if (!(exists(OpenNFS::TRACK_PATH))) {
            create_directories(OpenNFS::TRACK_PATH);
        }
    }
};

int main(int argc, char **argv) {
    auto logger = std::make_shared<Logger>();
    Config::get().InitFromCommandLine(argc, argv);

    try {
        OpenNFSEngine game(logger);
    } catch (const std::runtime_error &e) {
        LOG(WARNING) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
