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
    bool Render(float deltaTime, float totalTime, Camera &camera, ParamData &userParams, AssetData &loadedAssets, PhysicsEngine &physicsEngine);
    // Data used for culling
    uint32_t closestBlockID = 0;
private:
    GLFWwindow *window;
    std::shared_ptr<Logger> logger;
    std::vector<NeedForSpeed> installedNFSGames;
    shared_ptr<ONFSTrack> track;
    shared_ptr<Car> car;

    /* Renderers */
    TrackRenderer trackRenderer;
    CarRenderer carRenderer;
    SkyRenderer skyRenderer;
    ShadowMapRenderer shadowMapRenderer;

    /* Scene Objects */
    Light sun = Light(glm::vec3(0, 200, 0), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);
    Light moon = Light(glm::vec3(0, -200, 0), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);

    // ------- Helper Functions ------
    void InitialiseIMGUI();
    void InitGlobalLights();
    void NewFrame(ParamData &userParams);
    void UpdateShaders();
    bool UpdateGlobalLights(ParamData &userParams);
    std::vector<int> CullTrackBlocks(glm::vec3 worldPosition, int blockDrawDistance, bool useNeighbourData);
    void SetCulling(bool toCull);
    void DrawCarRaycasts(PhysicsEngine &physicsEngine);
    void DrawVroad(PhysicsEngine &physicsEngine);
    void DrawCameraAnimation(Camera &camera, PhysicsEngine &physicsEngine);
    void DrawDebugCube(PhysicsEngine &physicsEngine, glm::vec3 position);
    void DrawMetadata(Entity *targetEntity);
    void DrawNFS34Metadata(Entity *targetEntity);
    bool DrawMenuBar(AssetData &loadedAssets);
    void DrawUI(ParamData &userParams, Camera &camera);
};
