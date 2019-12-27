#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include "../Camera/FreeCamera.h"
#include "../Camera/HermiteCamera.h"
#include "../Scene/Entity.h"
#include "../Loaders/TrackLoader.h"
#include "../Physics/PhysicsEngine.h"
#include "../Loaders/CarLoader.h"
#include "../RaceNet/CarAgent.h"
#include "../Util/Logger.h"
#include "../Config.h"

#include "HermiteCurve.h"
#include "CarRenderer.h"
#include "TrackRenderer.h"
#include "SkyRenderer.h"
#include "ShadowMapRenderer.h"
/*#include "MenuRenderer.h"*/

class Renderer {
public:
    Renderer(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger, const std::vector<NfsAssetList> &installedNFS, const std::shared_ptr<ONFSTrack> &currentTrack, std::shared_ptr<Car> &currentCar);
    ~Renderer();
    static void GlfwError(int id, const char *description) {
        LOG(WARNING) << description;
    }
    static void WindowSizeCallback(GLFWwindow *window, int width, int height) {
        Config::get().resX = width;
        Config::get().resY = height;
    }
    static GLFWwindow *InitOpenGL(int resolutionX, int resolutionY, const std::string &windowName);
    bool Render(float totalTime,
                float deltaTime,
                FreeCamera &freeCamera,
                HermiteCamera &hermiteCamera,
                ParamData &userParams,
                AssetData &loadedAssets,
                std::shared_ptr<Car> &playerCar,
                const std::vector<CarAgent> &racers,
                PhysicsEngine &physicsEngine);

private:
    void _InitialiseIMGUI();
    void _NewFrame(ParamData &userParams);
    void _SetCamera(ParamData &userParams);
    void _DrawMetadata(Entity *targetEntity);
    void _DrawNFS34Metadata(Entity *targetEntity);
    bool _DrawMenuBar(AssetData &loadedAssets);
    void _DrawUI(ParamData &userParams, FreeCamera &freeCamera, std::shared_ptr<Car> &playerCar);

    // TODO: Move to debug Renderer class
    static void _DrawTrackCollision(std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine);
    static void _DrawAABB(const AABB &aabb, PhysicsEngine &physicsEngine);
    static void _DrawFrustum(Camera &camera, PhysicsEngine &physicsEngine);
    static void _DrawCarRaycasts(const std::shared_ptr<Car> &car, PhysicsEngine &physicsEngine);
    static void _DrawVroad(std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine);
    static void _DrawCameraAnimation(Camera &camera, std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine);

    GLFWwindow *m_pWindow;
    std::shared_ptr<Logger> m_logger;
    std::vector<NfsAssetList> m_nfsAssetList;
    std::shared_ptr<ONFSTrack> m_track;

    TrackRenderer m_trackRenderer;
    CarRenderer m_carRenderer;
    SkyRenderer m_skyRenderer;
    ShadowMapRenderer m_shadowMapRenderer;
    /*MenuRenderer menuRenderer;*/

    /* State */
    CameraMode m_activeCameraMode; // TODO: Change to a Camera base object
    bool m_entityTargeted = false;
    Entity *m_pTargetedEntity = nullptr;
    // Data used for culling
    int m_closestBlockID = 0;
};
