//
//  main.cpp
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 27/01/2015.
//
#include <cstdlib>
#include <string>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
/*#define GLFW_INCLUDE_VULKAN
#include "vkRenderer.h"*/

#define TINYOBJLOADER_IMPLEMENTATION
// Source
#include "Loaders/trk_loader.h"
#include "Loaders/car_loader.h"
#include "Loaders/music_loader.h"
#include "Physics/Car.h"
#include "Renderer/Renderer.h"
//#include <boost/log/trivial.hpp>

class OpenNFS {
public:
    void run() {
        std::cout << "----------- OpenNFS v0.1 -----------" << std::endl;
        // Must initialise OpenGL here as the Loaders instantiate meshes which create VAO's
        ASSERT(initOpenGL(), "OpenGL init failed.");
        initDirectories();
        std::vector<NeedForSpeed> installedNFS = populateAssets();

        /*------ ASSET LOAD ------*/
        //Load Track Data
        std::string nfs3ps1track("../resources/NFS_3_PS1/ZZZTR02B");
        std::string nfs2setrack("../resources/NFS_2_SE/GAMEDATA/TRACKS/SE/TR04");
        std::string nfs2track("../resources/NFS_2/GAMEDATA/TRACKS/PC/TR05");
        std::string nfs3track("../resources/NFS_3/gamedata/tracks/trk000");
        std::shared_ptr<ONFSTrack> track = TrackLoader::LoadTrack(nfs3track);

        //Load Car data from unpacked NFS files
        std::string merc("../resources/NFS_3/gamedata/carmodel/diab");
        std::string nfs4("../resources/NFS_4/DATA/CARS/LDIA");
        std::string nfs3ps1car("../resources/NFS_3_PS1/ZDIAB");
        std::string nfs2car("../resources/NFS_2/GAMEDATA/CARMODEL/PC/GT90");
        std::shared_ptr<Car> car = CarLoader::LoadCar(nfs4);

        //Load Music
        //MusicLoader musicLoader("F:\\NFS3\\nfs3_modern_base_eng\\gamedata\\audio\\pc\\hometech");

        AssetData loadedAssets = {
                NFS_4, "LDIAB",
                NFS_3, "trk000"
        };

        /*------- Render --------*/
        while(loadedAssets.trackTag != UNKNOWN){
            Renderer renderer(window, installedNFS, track, car);
            loadedAssets = renderer.Render();
        }

        /*vkRenderer renderer;

        try {
            renderer.run();
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }*/
    }

private:
    GLFWwindow *window;

    static void glfwError(int id, const char *description) {
        std::cout << description << std::endl;
    }

    bool initOpenGL() {
        // Initialise GLFW
        ASSERT(glfwInit(), "GLFW Init failed.\n");
        glfwSetErrorCallback(&glfwError);
        glfwWindowHint(GLFW_SAMPLES, 4);
        // TODO: If we fail to create a GL context, fall back to not requesting any (Keiiko Bug #1)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

        window = glfwCreateWindow(1024, 768, "OpenNFS", nullptr, nullptr);

        if (window == nullptr) {
            fprintf(stderr, "Failed to create a GLFW window.\n");
            getchar();
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);

        // Initialize GLEW
        glewExperimental = GL_TRUE; // Needed for core profile

        if (glewInit() != GLEW_OK) {
            fprintf(stderr, "Failed to initialize GLEW\n");
            getchar();
            glfwTerminate();
            return false;
        }

        // Ensure we can capture the escape key being pressed below
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        // Set the mouse at the center of the screen
        glfwPollEvents();
        glfwSetCursorPos(window, 1024 / 2, 768 / 2);

        // Dark blue background
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        // Accept fragment if it closer to the camera than the former one
        glDepthFunc(GL_LESS);

        // Cull triangles which normal is not towards the camera (when culling is enabled)
        glFrontFace(GL_CCW);
        glEnable(GL_BACK);
        //glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        return true;
    }

    void initDirectories() {
        if (!(boost::filesystem::exists(CAR_PATH))) {
            boost::filesystem::create_directories(CAR_PATH);
        }
        if (!(boost::filesystem::exists(TRACK_PATH))) {
            boost::filesystem::create_directories(TRACK_PATH);
        }
    }

    std::vector<NeedForSpeed> populateAssets(){
        boost::filesystem::path basePath("../resources/");
        std::vector<NeedForSpeed> installedNFS;
        bool hasLanes = false;
        bool hasMisc = false;
        bool hasSfx = false;

        for (boost::filesystem::directory_iterator itr(basePath); itr != boost::filesystem::directory_iterator(); ++itr) {
            NeedForSpeed currentNFS;
            currentNFS.tag = UNKNOWN;

            if (itr->path().filename().string().find(ToString(NFS_2_SE)) != std::string::npos) {
                currentNFS.tag = NFS_2_SE;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_2_SE_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                ASSERT(boost::filesystem::exists(trackBasePath), "NFS 2 Special Edition track folder: " << trackBasePath << " is missing.");

                for (boost::filesystem::directory_iterator itr(trackBasePath); itr != boost::filesystem::directory_iterator(); ++itr) {
                    if (itr->path().filename().string().find(".TRK") != std::string::npos) {
                        currentNFS.tracks.emplace_back(itr->path().filename().replace_extension("").string());
                    }
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_2_SE_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                ASSERT(boost::filesystem::exists(carBasePath), "NFS 2 Special Edition car folder: " << carBasePath << " is missing.");

                // TODO: Work out where NFS2 SE Cars are stored
                continue;
            } else if (itr->path().filename().string().find(ToString(NFS_2)) != std::string::npos) {
                currentNFS.tag = NFS_2;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_2_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                ASSERT(boost::filesystem::exists(trackBasePath), "NFS 2 track folder: " << trackBasePath << " is missing.");

                for (boost::filesystem::directory_iterator trackItr(trackBasePath); trackItr != boost::filesystem::directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".TRK") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_2_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                ASSERT(boost::filesystem::exists(carBasePath), "NFS 2 car folder: " << carBasePath << " is missing.");

                for (boost::filesystem::directory_iterator carItr(carBasePath); carItr != boost::filesystem::directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".GEO") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string().find(ToString(NFS_3_PS1)) != std::string::npos) {
                currentNFS.tag = NFS_3_PS1;

                for (boost::filesystem::directory_iterator trackItr(itr->path().string()); trackItr != boost::filesystem::directory_iterator(); ++trackItr) {
                    if (trackItr->path().filename().string().find(".TRK") != std::string::npos) {
                        currentNFS.tracks.emplace_back(trackItr->path().filename().replace_extension("").string());
                    }
                }

                for (boost::filesystem::directory_iterator carItr(itr->path().string()); carItr != boost::filesystem::directory_iterator(); ++carItr) {
                    if (carItr->path().filename().string().find(".GEO") != std::string::npos) {
                        currentNFS.cars.emplace_back(carItr->path().filename().replace_extension("").string());
                    }
                }
            } else if (itr->path().filename().string().find(ToString(NFS_3)) != std::string::npos) {
                currentNFS.tag = NFS_3;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_3_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                ASSERT(boost::filesystem::exists(trackBasePath), "NFS 3 Hot Pursuit track folder: " << trackBasePath << " is missing.");

                // TODO: Dive one folder deeper, or alter the loader to be able to handle just the base path
                for (boost::filesystem::directory_iterator trackItr(trackBasePath); trackItr != boost::filesystem::directory_iterator(); ++trackItr) {
                    currentNFS.tracks.emplace_back(trackItr->path().filename().string());
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_3_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                ASSERT(boost::filesystem::exists(carBasePath), "NFS 3 Hot Pursuit car folder: " << carBasePath << " is missing.");

                for (boost::filesystem::directory_iterator carItr(carBasePath); carItr != boost::filesystem::directory_iterator(); ++carItr) {
                        currentNFS.cars.emplace_back(carItr->path().filename().string());
                }
            } else if (itr->path().filename().string().find(ToString(NFS_4)) != std::string::npos) {
                currentNFS.tag = NFS_4;

                std::stringstream trackBasePathStream;
                trackBasePathStream << itr->path().string() << NFS_4_TRACK_PATH;
                std::string trackBasePath(trackBasePathStream.str());
                ASSERT(boost::filesystem::exists(trackBasePath), "NFS 4 High Stakes track folder: " << trackBasePath << " is missing.");

                // TODO: Dive one folder deeper, or alter the loader to be able to handle just the base path
                for (boost::filesystem::directory_iterator trackItr(trackBasePath); trackItr != boost::filesystem::directory_iterator(); ++trackItr) {
                    currentNFS.tracks.emplace_back(trackItr->path().filename().string());
                }

                std::stringstream carBasePathStream;
                carBasePathStream << itr->path().string() << NFS_4_CAR_PATH;
                std::string carBasePath(carBasePathStream.str());
                ASSERT(boost::filesystem::exists(carBasePath), "NFS 4 High Stakes car folder: " << carBasePath << " is missing.");

                for (boost::filesystem::directory_iterator carItr(carBasePath); carItr != boost::filesystem::directory_iterator(); ++carItr) {
                    currentNFS.cars.emplace_back(carItr->path().filename().string());
                }
            } else if (itr->path().filename().string().find("lanes") != std::string::npos) {
                hasLanes = true;
                continue;
            } else if (itr->path().filename().string().find("misc") != std::string::npos) {
                hasMisc = true;
                continue;
            } else if (itr->path().filename().string().find("sfx") != std::string::npos) {
                hasSfx = true;
                continue;
            } else {
                std::cerr << "Unknown folder in resources directory: " << itr->path().filename().string() << std::endl;
                continue;
            }
            installedNFS.emplace_back(currentNFS);
        }

        ASSERT(hasLanes, "Missin \'lanes\' folder in resources directory");
        ASSERT(hasMisc, "Missin \'misc\' folder in resources directory");
        ASSERT(hasSfx, "Missin \'sfx\' folder in resources directory");

        return installedNFS;
    }
};

int main(int argc, char **argv) {
    OpenNFS game;

    try {
        game.run();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
