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

using namespace ImGui;

class OpenNFS {
public:
    void run() {
        std::cout << "----------- OpenNFS v0.1 -----------" << std::endl;
        // Must initialise OpenGL here as the Loaders instantiate meshes which create VAO's
        ASSERT(initOpenGL(), "OpenGL init failed.");
        initDirectories();

        /*------ ASSET LOAD ------*/
        //Load Track Data
        std::string nfs3ps1track("../resources/NFS3_PS1/ZZZTR00A");
        std::string nfs2track("../resources/NFS2/GAMEDATA/TRACKS/PC/TR03");
        std::string nfs3track("../resources/NFS3/gamedata/tracks/trk006/tr06");
        std::shared_ptr<ONFSTrack> track = TrackLoader::LoadTrack(nfs3track);
        //Load Car data from unpacked NFS files
        std::string merc("../resources/NFS3/gamedata/carmodel/merc");
        std::string nfs4("../resources/NFS4/DATA/CARS/B911");
        std::string nfs3ps1car("../resources/NFS3_PS1/ZDIAB");
        std::string nfs2car("../resources/NFS2/GAMEDATA/CARMODEL/PC/GT90");
        std::shared_ptr<Car> car = CarLoader::LoadCar(nfs4);

        //Load Music
        //MusicLoader musicLoader("F:\\NFS3\\nfs3_modern_base_eng\\gamedata\\audio\\pc\\hometech");

        /*------- Render --------*/
        Renderer renderer(window, track, car);
        renderer.Render();

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
