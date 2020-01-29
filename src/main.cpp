#define TINYOBJLOADER_IMPLEMENTATION

#ifdef VULKAN_BUILD
#define GLFW_INCLUDE_VULKAN

#include "Renderer/vkRenderer.h"

#endif

#include <cstdlib>
#include <string>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Config.h"
#include "Util/Logger.h"
#include "Scene/Track.h"
#include "Loaders/TrackLoader.h"
#include "Loaders/CarLoader.h"
#include "Loaders/MusicLoader.h"
#include "Renderer/Renderer.h"
#include "Race/RaceSession.h"
#include "RaceNet/TrainingGround.h"

class OpenNFS {
public:
    explicit OpenNFS(std::shared_ptr<Logger> &onfs_logger) : logger(onfs_logger) {
        InitDirectories();
        PopulateAssets();

        if (Config::get().vulkanRender) {
#ifdef VULKAN_BUILD
            vkRenderer renderer;
            renderer.run();
#else
            ASSERT(false, "This build of OpenNFS was not compiled with Vulkan support!");
#endif
        } else if (Config::get().trainingMode) {
            train();
        } else {
            run();
        }
    }

    void run() {
        LOG(INFO) << "OpenNFS Version " << ONFS_VERSION;

        // Must initialise OpenGL here as the Loaders instantiate meshes which create VAO's
        std::shared_ptr<GLFWwindow> window = Renderer::InitOpenGL(Config::get().resX, Config::get().resY, "OpenNFS v" + ONFS_VERSION);

        AssetData loadedAssets = {
                getEnum(Config::get().carTag), Config::get().car,
                getEnum(Config::get().trackTag), Config::get().track
        };

        // TODO: TEMP FIX UNTIL I DO A PROPER RETURN from race session
        ASSERT(loadedAssets.trackTag != UNKNOWN, "Unknown track type!");

        /*------- Render --------*/
        while (loadedAssets.trackTag != UNKNOWN) {
            /*------ ASSET LOAD ------*/
            //Load Car data from unpacked NFS files
            auto car = CarLoader::LoadCar(loadedAssets.carTag, loadedAssets.car);
            //Load Track Data
            auto track = TrackLoader::LoadTrack(loadedAssets.trackTag, loadedAssets.track);

            //Load Music
            //MusicLoader musicLoader("F:\\NFS3\\nfs3_modern_base_eng\\gamedata\\audio\\pc\\atlatech");

            RaceSession race(window, logger, installedNFS, track, car);
            loadedAssets = race.Simulate();
        }

        // Close OpenGL window and terminate GLFW
        glfwTerminate();
    }

    void train() {
        LOG(INFO) << "OpenNFS Version " << ONFS_VERSION << " (GA Training Mode)";

        // Must initialise OpenGL here as the Loaders instantiate meshes which create VAO's
        std::shared_ptr<GLFWwindow> window = Renderer::InitOpenGL(Config::get().resX, Config::get().resY, "OpenNFS v" + ONFS_VERSION + " (GA Training Mode)");

        AssetData trainingAssets = {
                getEnum(Config::get().carTag), Config::get().car,
                getEnum(Config::get().trackTag), Config::get().track
        };

        /*------ ASSET LOAD ------*/
        //Load TrackModel Data
        auto track = TrackLoader::LoadTrack(trainingAssets.trackTag, trainingAssets.track);
        //Load Car data from unpacked NFS files
        std::shared_ptr<Car> car = CarLoader::LoadCar(trainingAssets.carTag, trainingAssets.car);

        auto trainingGround = TrainingGround(Config::get().nGenerations,
                                             Config::get().nTicks, track, car, logger, window);
    }

private:
    std::shared_ptr<Logger> logger;

    std::vector<NfsAssetList> installedNFS;

    static void InitDirectories() {
        if (!(boost::filesystem::exists(CAR_PATH))) {
            boost::filesystem::create_directories(CAR_PATH);
        }
        if (!(boost::filesystem::exists(TRACK_PATH))) {
            boost::filesystem::create_directories(TRACK_PATH);
        }
    }

    void PopulateAssets() {
        using namespace boost::filesystem;

        path basePath(RESOURCE_PATH);
        bool hasLanes = false;
        bool hasMisc = false;
        bool hasSfx = false;
        bool hasUI = false;

        for (directory_iterator itr(basePath); itr != directory_iterator(); ++itr) {
            NfsAssetList currentNFS;
            currentNFS.tag = UNKNOWN;

            if (itr->path().filename().string() == ToString(NFS_2_SE)) {
                currentNFS.tag = NFS_2_SE;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_2_SE_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                ASSERT(exists(trackBasePath), "NFS 2 Special Edition track folder: " << trackBasePath << " is missing.");

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".TRK") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_2_SE_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                ASSERT(exists(carBasePath), "NFS 2 Special Edition car folder: " << carBasePath << " is missing.");

                // TODO: Work out where NFS2 SE Cars are stored
            } else if (itr->path().filename().string() == ToString(NFS_2)) {
                currentNFS.tag = NFS_2;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_2_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                ASSERT(exists(trackBasePath), "NFS 2 track folder: " << trackBasePath << " is missing.");

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".TRK") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_2_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                ASSERT(exists(carBasePath), "NFS 2 car folder: " << carBasePath << " is missing.");

                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".GEO") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == ToString(NFS_2_PS1)) {
                currentNFS.tag = NFS_2_PS1;

                for (directory_iterator trackItr(itr->path().string()); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".TRK") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                for (directory_iterator carItr(itr->path().string()); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".GEO") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == ToString(NFS_3_PS1)) {
                currentNFS.tag = NFS_3_PS1;

                for (directory_iterator trackItr(itr->path().string()); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".TRK") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                for (directory_iterator carItr(itr->path().string()); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".GEO") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == ToString(NFS_3)) {
                currentNFS.tag = NFS_3;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_3_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                ASSERT(exists(trackBasePath), "NFS 3 Hot Pursuit track folder: " << trackBasePath << " is missing.");

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    currentNFS.tracks.emplace_back(trackItr->path().filename().string());
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_3_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                ASSERT(exists(carBasePath), "NFS 3 Hot Pursuit car folder: " << carBasePath << " is missing.");

                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find("traffic") == std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().string());
                    }
                }

                carBasePathStream << "traffic/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    currentNFS.cars.emplace_back("traffic/" + carItr->path().filename().string());
                }

                carBasePathStream << "pursuit/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find("PURSUIT") == std::string::npos) {
                        currentNFS.cars.emplace_back("traffic/pursuit/" + carItr->path().filename().string());
                    }
                }
            } else if (itr->path().filename().string() == ToString(NFS_4_PS1)) {
                currentNFS.tag = NFS_4_PS1;

                for (directory_iterator dirItr(itr->path().string()); dirItr != directory_iterator(); ++dirItr) {
                    if (dirItr->path().filename().string().find("ZZZ") == 0 && dirItr->path().filename().string().find(".VIV") != std::string::npos) {
                        currentNFS.cars.emplace_back(dirItr->path().filename().replace_extension("").string());
                    } else if (dirItr->path().filename().string().find("ZTR") == 0 && dirItr->path().filename().string().find(".GRP") != std::string::npos){
                        currentNFS.tracks.emplace_back(dirItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == ToString(NFS_4)) {
                currentNFS.tag = NFS_4;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_4_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                ASSERT(exists(trackBasePath), "NFS 4 High Stakes track folder: " << trackBasePath << " is missing.");

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    currentNFS.tracks.emplace_back(trackItr->path().filename().string());
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_4_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                ASSERT(exists(carBasePath), "NFS 4 High Stakes car folder: " << carBasePath << " is missing.");

                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find("TRAFFIC") == std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().string());
                    }
                }

                carBasePathStream << "TRAFFIC/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    if ((carItr->path().filename().string().find("CHOPPERS") == std::string::npos) &&
                        (carItr->path().filename().string().find("PURSUIT") == std::string::npos)) {
                        currentNFS.cars.emplace_back("TRAFFIC/" + carItr->path().filename().string());
                    }
                }

                carBasePathStream << "CHOPPERS/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    currentNFS.cars.emplace_back("TRAFFIC/CHOPPERS/" + carItr->path().filename().string());
                }

                carBasePathStream.str(std::string());
                carBasePathStream << itr->path().string() << NFS_4_CAR_PATH << "TRAFFIC/" << "PURSUIT/";
                for (directory_iterator carItr(carBasePathStream.str()); carItr != directory_iterator(); ++carItr) {
                    currentNFS.cars.emplace_back("TRAFFIC/PURSUIT/" + carItr->path().filename().string());
                }
            }  else if (itr->path().filename().string() == ToString(MCO)) {
                currentNFS.tag = MCO;

                std::string trackBasePath = itr->path().string() + MCO_TRACK_PATH;
                ASSERT(exists(trackBasePath), "Motor City Online track folder: " << trackBasePath << " is missing.");

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    currentNFS.tracks.emplace_back(trackItr->path().filename().string());
                }

                std::string carBasePath = itr->path().string() + MCO_CAR_PATH;
                ASSERT(exists(carBasePath), "Motor City Online car folder: " << carBasePath << " is missing.");
                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                }
            } else if (itr->path().filename().string() == ToString(NFS_5)) {
                currentNFS.tag = NFS_5;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_5_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                ASSERT(exists(trackBasePath), "NFS 5 track folder: " << trackBasePath << " is missing.");

                for (directory_iterator trackItr(trackBasePath); trackItr != directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".crp") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_5_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                ASSERT(exists(carBasePath), "NFS 5 car folder: " << carBasePath << " is missing.");

                for (directory_iterator carItr(carBasePath); carItr != directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".crp") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string() == "lanes") {
                hasLanes = true;
                continue;
            } else if (itr->path().filename().string() == "misc") {
                hasMisc = true;
                continue;
            } else if (itr->path().filename().string() == "sfx") {
                hasSfx = true;
                continue;
            } else if (itr->path().filename().string() == "ui") {
                hasUI = true;
                continue;
            } else if (itr->path().filename().string() == "asset") {
                continue;
            } else {
                LOG(WARNING) << "Unknown folder in resources directory: " << itr->path().filename().string();
                continue;
            }
            installedNFS.emplace_back(currentNFS);
        }

        ASSERT(hasLanes, "Missing \'lanes\' folder in resources directory");
        ASSERT(hasMisc, "Missing \'misc\' folder in resources directory");
        ASSERT(hasSfx, "Missing \'sfx\' folder in resources directory");
        ASSERT(hasUI, "Missing \'ui\' folder in resources directory");
        ASSERT(installedNFS.size(), "No Need for Speed games detected in resources directory");

        for (auto nfs : installedNFS) {
            LOG(INFO) << "Detected: " << ToString(nfs.tag);
        }
    }
};

int main(int argc, char **argv) {
    Config::get().InitFromCommandLine(argc, argv);
    std::shared_ptr<Logger> logger = std::make_shared<Logger>();

    try {
        OpenNFS game(logger);
    } catch (const std::runtime_error &e) {
        LOG(WARNING) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
