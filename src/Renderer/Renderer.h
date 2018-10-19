//
// Created by Amrik Sadhra on 19/07/2018.
//

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <g3log/g3log.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include "../Scene/Camera.h"
#include "../Scene/Entity.h"
#include "../Loaders/trk_loader.h"
#include "../Physics/Physics.h"
#include "../Config.h"

#include "HermiteCurve.h"
#include "CarRenderer.h"
#include "TrackRenderer.h"
#include "SkyRenderer.h"
#include "../Shaders/DepthShader.h"

class Renderer {
public:
    Renderer(GLFWwindow *gl_window, const std::vector<NeedForSpeed> &installedNFS, const shared_ptr<ONFSTrack> &current_track, shared_ptr<Car> current_car);
    ~Renderer();
    AssetData Render();
private:
    GLFWwindow *window;
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

    /* Shadow Mapping */
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    GLuint depthMapFBO = 0;
    GLuint depthTextureID = 0;
    DepthShader depthShader;

    /* Scene Objects */
    Camera mainCamera;
    Light cameraLight;

    // ------ Renderer State ------
    uint64_t ticks = 0; // Engine ticks elapsed
    float totalTime = 0;

    // Data used for culling
    int closestBlockID = 0;

    // ------- Helper Functions ------
    void SetCulling(bool toCull);
    void DrawDebugCube(glm::vec3 position);
    void DrawMetadata(Entity *targetEntity);
    void DrawNFS34Metadata(Entity *targetEntity);
    bool DrawMenuBar();
    void InitialiseDepthTexture();
    void DrawUI(ParamData *preferences, glm::vec3 worldPositions);
    void NewFrame(ParamData *userParams);
    std::vector<int> CullTrackBlocks(glm::vec3 oldWorldPosition, glm::vec3 worldPosition, int blockDrawDistance, bool useNeighbourData);
    Entity *CheckForPicking(glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, bool *entity_targeted);
};
