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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <examples/opengl3_example/imgui_impl_glfw_gl3.h>
#include <set>
/*#define GLFW_INCLUDE_VULKAN
#include "vkRenderer.h"*/

#define TINYOBJLOADER_IMPLEMENTATION
// Source
#include "Util/Utils.h"
#include "Loaders/trk_loader.h"
#include "Loaders/car_loader.h"
#include "Loaders/music_loader.h"
#include "Physics/Physics.h"
#include "Physics/Car.h"
#include "Renderer/Renderer.h"

using namespace ImGui;

class OpenNFS {
public:
    void run() {
        std::cout << "----------- OpenNFS3 v0.1 -----------" << std::endl;
        ASSERT(initOpenGL(), "OpenGL init failed.");
        /*------ ASSET LOAD ------*/
        initDirectories();
        //Load Track Data`
        std::shared_ptr<ONFSTrack> track = TrackLoader::LoadTrack("../resources/NFS3/gamedata/tracks/trk006/tr06");
        //Load Car data from unpacked NFS files
        std::shared_ptr<Car> car = CarLoader::LoadCar("../resources/NFS3/gamedata/carmodel/diab");

        //Load Music
        //MusicLoader musicLoader("F:\\NFS3\\nfs3_modern_base_eng\\gamedata\\audio\\pc\\hometech");

        /*------- Render --------*/
        Renderer renderer(window, track, car);
        renderer.Render();
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

        window = glfwCreateWindow(1024, 768, "OpenNFS3", nullptr, nullptr);

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

    int launchVK() {
        /*vkRenderer renderer;

        try {
            renderer.run();
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }*/

        return EXIT_SUCCESS;
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
