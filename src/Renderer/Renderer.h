//
// Created by SYSTEM on 19/07/2018.
//

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <examples/opengl3_example/imgui_impl_glfw_gl3.h>

#include "../Shaders/TrackShader.h"
#include "../Shaders/CarShader.h"
#include "../Shaders/BillboardShader.h"
#include "../Scene/Camera.h"
#include "../Loaders/trk_loader.h"
#include "../Physics/Physics.h"
#include "HermiteCurve.h"

struct ParamData {
    ImVec4 clear_color = ImVec4(119 / 255.0f, 197 / 255.0f, 252 / 255.0f, 1.0f);
    ImVec4 car_color = ImVec4(247 / 255.0f, 203 / 255.0f, 32 / 255.0f, 1.0f);
    ImVec4 test_light_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    float carSpecReflectivity = 1;
    float carSpecDamper = 10;
    float trackSpecReflectivity = 1;
    float trackSpecDamper = 10;
    int blockDrawDistance = 15;
    bool window_active = true;
    bool physics_debug_view = false;
    bool use_classic_graphics = true;
    bool attach_cam_to_hermite = false;
    bool use_nb_data = true;
};

class Renderer {
public:
    Renderer(GLFWwindow *gl_window, shared_ptr<ONFSTrack> current_track, shared_ptr<Car> current_car);
    ~Renderer();
    void Render();
private:
    GLFWwindow *window;
    shared_ptr<ONFSTrack> track;
    shared_ptr<Car> car;


    /*------- BULLET --------*/
    Physics physicsEngine;

    // Create and compile our GLSL programs from the shaders
    TrackShader trackShader;
    CarShader carShader;
    BillboardShader billboardShader;

    Camera mainCamera;
    Light cameraLight;

    // ------ Renderer State ------
    // Data used for culling
    int closestBlockID = 0;


    // ------- Helper Functions ------
    void SetCulling(bool toCull);
    void DrawDebugCube(glm::vec3 position);
    void DrawMenuBar();
    void DrawUI(ParamData *preferences, glm::vec3 worldPositions);
    void NewFrame(bool &window_active);
    std::vector<int> CullTrackBlocks(glm::vec3 oldWorldPosition, glm::vec3 worldPosition, int blockDrawDistance, bool useNeighbourData);
    void CheckForPicking(glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix);
};
