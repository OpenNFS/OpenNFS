//
// Created by Amrik Sadhra on 19/07/2018.
//

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <chrono>
#include <thread>

#include "../Scene/Camera.h"
#include "../Scene/Entity.h"
#include "../Loaders/trk_loader.h"
#include "../Physics/PhysicsEngine.h"
#include "../Loaders/car_loader.h"
#include "../RaceNet/CarAgent.h"
#include "../Util/Logger.h"
#include "../Config.h"

#include "HermiteCurve.h"
#include "CarRenderer.h"
#include "TrackRenderer.h"
#include "SkyRenderer.h"
#include "ShadowMapRenderer.h"

class Renderer {
public:
    Renderer(GLFWwindow *gl_window, std::shared_ptr<Logger> &onfs_logger, const std::vector<NeedForSpeed> &installedNFS, const shared_ptr<ONFSTrack> &current_track, shared_ptr<Car> &current_car);
    ~Renderer();
    AssetData Render();
private:
    GLFWwindow *window;
    std::shared_ptr<Logger> logger;
    std::vector<NeedForSpeed> installedNFSGames;
    AssetData loadedAssets;
    shared_ptr<ONFSTrack> track;
    shared_ptr<Car> car;

    /*------- BULLET --------*/
    PhysicsEngine physicsEngine;

    /* Renderers */
    TrackRenderer trackRenderer;
    CarRenderer carRenderer;
    SkyRenderer skyRenderer;
    ShadowMapRenderer shadowMapRenderer;

    /* Scene Objects */
    Camera mainCamera;
    Light sun = Light(glm::vec3(0, 200, 0), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);
    Light moon = Light(glm::vec3(0, -200, 0), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);

    // ------ Renderer State ------
    uint64_t ticks = 0; // Engine ticks elapsed
    float totalTime = 0;

    // Data used for culling
    uint32_t closestBlockID = 0;

    // ------- Helper Functions ------
    void InitialiseIMGUI();
    void InitGlobalLights();
    void NewFrame(ParamData *userParams);
    void UpdateShaders();
    bool UpdateGlobalLights(ParamData *userParams );
    std::vector<int> CullTrackBlocks(glm::vec3 worldPosition, int blockDrawDistance, bool useNeighbourData);
    void SetCulling(bool toCull);
    void DrawCarRaycasts();
    void DrawVroad();
    void DrawCameraAnimation();
    void DrawDebugCube(glm::vec3 position);
    void DrawMetadata(Entity *targetEntity);
    void DrawNFS34Metadata(Entity *targetEntity);
    bool DrawMenuBar();
    void DrawUI(ParamData *preferences, glm::vec3 worldPositions);
};
