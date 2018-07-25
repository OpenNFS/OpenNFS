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

class Renderer {
public:
    Renderer(GLFWwindow *gl_window, shared_ptr<ONFSTrack> current_track, shared_ptr<Car> current_car);
    ~Renderer();
    void render();
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
    // Cam Spline
    HermiteCurve cameraSpline;
    int loopTime;
    // Data used for culling
    int closestBlockID = 0;


    // ------- Helper Functions ------
    void DrawDebugCube(glm::vec3 position);
    void DrawMenuBar();
    void newFrame(bool &window_active);
    std::vector<TrackBlock> CullTrackBlocks(glm::vec3 oldWorldPosition, glm::vec3 worldPosition, int blockDrawDistance);
};
