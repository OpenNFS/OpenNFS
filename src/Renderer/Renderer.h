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

#include "../Scene/Camera.h"
#include "../Scene/Entity.h"
#include "../Loaders/trk_loader.h"
#include "../Physics/Physics.h"
#include "../Loaders/car_loader.h"
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
    static void ResetToVroad(uint32_t trackBlockIndex, std::shared_ptr<ONFSTrack> &track, std::shared_ptr<Car> &car); // TODO: Move this _somewhere_
private:
    GLFWwindow *window;
    std::shared_ptr<Logger> logger;
    std::vector<NeedForSpeed> installedNFSGames;
    AssetData loadedAssets;
    shared_ptr<ONFSTrack> track;
    shared_ptr<Car> car;

    /*------- BULLET --------*/
    Physics physicsEngine;

    /* Renderers */
    TrackRenderer trackRenderer;
    CarRenderer carRenderer;
    SkyRenderer skyRenderer;
    ShadowMapRenderer shadowMapRenderer;

    /* Scene Objects */
    Camera mainCamera;
    Light cameraLight;
    Light sun = Light(glm::vec3(0, 200, 0), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);
    Light moon = Light(glm::vec3(0, -200, 0), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);

    // ------ Renderer State ------
    uint64_t ticks = 0; // Engine ticks elapsed
    float totalTime = 0;

    // Data used for culling
    int closestBlockID = 0;

    // ------- Helper Functions ------
    void SetCulling(bool toCull);
    void DrawCarRaycasts();
    void DrawVroad();
    void DrawCameraAnimation();
    void DrawDebugCube(glm::vec3 position);
    void InitialiseIMGUI();
    void DrawMetadata(Entity *targetEntity);
    void DrawNFS34Metadata(Entity *targetEntity);
    bool DrawMenuBar();
    void DrawUI(ParamData *preferences, glm::vec3 worldPositions);
    void NewFrame(ParamData *userParams);
    std::vector<int> CullTrackBlocks(glm::vec3 oldWorldPosition, glm::vec3 worldPosition, int blockDrawDistance, bool useNeighbourData);
    Entity *CheckForPicking(glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, bool *entity_targeted);
};
