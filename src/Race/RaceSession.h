#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Camera/FreeCamera.h"
#include "../Camera/HermiteCamera.h"
#include "../Camera/CarCamera.h"
#include "../Physics/PhysicsEngine.h"
#include "../Loaders/CarLoader.h"
#include "../Loaders/TrackLoader.h"
#include "../Renderer/Renderer.h"
#include "../RaceNet/Agents/PlayerAgent.h"
#include "../Util/Logger.h"
#include "../Config.h"
#include "RacerManager.h"

class RaceSession {
public:
    RaceSession(GLFWwindow *glWindow,
                std::shared_ptr<Logger> &onfsLogger,
                const std::vector<NfsAssetList> &installedNFS,
                std::shared_ptr<ONFSTrack> currentTrack,
                std::shared_ptr<Car> &currentCar);
    AssetData Simulate();

private:
    std::shared_ptr<Camera> _GetActiveCamera();
    void _UpdateCameras(float deltaTime);
    void _GetInputsAndClear();

    GLFWwindow *m_pWindow;
    AssetData m_loadedAssets;
    WindowStatus m_windowStatus = WindowStatus::UI;

    std::shared_ptr<ONFSTrack> m_track;
    PlayerAgent m_playerAgent;

    PhysicsEngine m_physicsEngine;
    Renderer m_renderer;
    RacerManager m_racerManager;
    CameraMode m_activeCameraMode = CameraMode::FREE_LOOK;
    std::shared_ptr<FreeCamera> m_freeCamera;
    std::shared_ptr<HermiteCamera> m_hermiteCamera;
    std::shared_ptr<CarCamera> m_carCamera;

    ParamData m_userParams;
    uint64_t m_ticks = 0; // Engine ticks elapsed
    float m_totalTime = 0;
};
