//
// Created by Amrik Sadhra on 19/07/2018.
//

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <examples/opengl3_example/imgui_impl_glfw_gl3.h>

#include "../Scene/Camera.h"
#include "../Scene/Entity.h"
#include "../Loaders/trk_loader.h"
#include "../Physics/Physics.h"
#include "../Config.h"

#include "HermiteCurve.h"
#include "CarRenderer.h"
#include "TrackRenderer.h"

class Renderer {
public:
    Renderer(GLFWwindow *gl_window, const shared_ptr<ONFSTrack> &current_track, shared_ptr<Car> current_car);
    ~Renderer();
    void Render();
private:
    GLFWwindow *window;
    shared_ptr<ONFSTrack> track;
    shared_ptr<Car> car;

    /*------- BULLET --------*/
    Physics physicsEngine;

    TrackRenderer trackRenderer;
    CarRenderer carRenderer;

    Camera mainCamera;
    Light cameraLight;

    // ------ Renderer State ------
    // Data used for culling
    int closestBlockID = 0;

    // ------- Helper Functions ------
    void SetCulling(bool toCull);
    void DrawDebugCube(glm::vec3 position);
    void DrawMetadata(Entity *targetEntity);
    void DrawNFS34Metadata(Entity *targetEntity);
    void DrawMenuBar();
    void DrawUI(ParamData *preferences, glm::vec3 worldPositions);
    void NewFrame(ParamData *userParams);
    std::vector<int> CullTrackBlocks(glm::vec3 oldWorldPosition, glm::vec3 worldPosition, int blockDrawDistance, bool useNeighbourData);
    Entity *CheckForPicking(glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, bool *entity_targeted);
};
