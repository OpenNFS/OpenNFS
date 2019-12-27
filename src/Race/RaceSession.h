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
#include "../RaceNet/CarAgent.h"
#include "../Util/Logger.h"
#include "../Config.h"

class RaceSession {
public:
    RaceSession(GLFWwindow *glWindow,
                std::shared_ptr<Logger> &onfsLogger,
                const std::vector<NfsAssetList> &installedNFS,
                const std::shared_ptr<ONFSTrack> &currentTrack,
                std::shared_ptr<Car> &currentCar);
    AssetData Simulate();

private:
    Camera _GetCamera();
    void _UpdateCameras(float deltaTime);
    void _SpawnRacers(uint8_t nRacers);
    void _GetInputsAndClear();

    GLFWwindow *m_pWindow;
    AssetData m_loadedAssets;
    WindowStatus m_windowStatus = WindowStatus::UI;

    std::shared_ptr<ONFSTrack> m_track;
    std::shared_ptr<Car> m_playerCar;
    std::vector<CarAgent> m_racerCars;

    PhysicsEngine m_physicsEngine;
    Renderer m_renderer;
    CameraMode m_activeCameraMode;
    FreeCamera m_freeCamera;
    HermiteCamera m_hermiteCamera;
    CarCamera m_carCamera;

    ParamData m_userParams;
    uint64_t m_ticks = 0; // Engine ticks elapsed
    float m_totalTime = 0;


};
