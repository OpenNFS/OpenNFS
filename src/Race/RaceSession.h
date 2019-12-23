#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Scene/Camera.h"
#include "../Physics/PhysicsEngine.h"
#include "../Loaders/CarLoader.h"
#include "../Loaders/TrackLoader.h"
#include "../RaceNet/CarAgent.h"
#include "../Util/Logger.h"
#include "../Config.h"
#include "../Renderer/Renderer.h"

class RaceSession {
public:
    RaceSession(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger, const std::vector<NfsAssetList> &installedNFS, const std::shared_ptr<ONFSTrack> &currentTrack, std::shared_ptr<Car> &currentCar);
    AssetData Simulate();
private:
    void SpawnRacers(uint8_t nRacers);

    GLFWwindow *m_window;
    AssetData m_loadedAssets;

    std::shared_ptr<ONFSTrack> m_track;
    std::shared_ptr<Car> m_playerCar;
    std::vector<CarAgent> m_racerCars;

    PhysicsEngine m_physicsEngine;
    Renderer m_renderer;
    Camera m_mainCamera;

    ParamData m_userParams;
    uint64_t m_ticks = 0; // Engine ticks elapsed
    float m_totalTime = 0;
};
