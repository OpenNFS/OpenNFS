#include "RaceSession.h"

RaceSession::RaceSession(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger, const std::vector<NfsAssetList > &installedNFS,
                         const std::shared_ptr<ONFSTrack> &currentTrack, std::shared_ptr<Car>
                         &currentCar) : m_window(glWindow), m_track(currentTrack), m_playerCar(currentCar),
                                        m_renderer(glWindow, onfsLogger, installedNFS, m_track, m_playerCar) {

    m_loadedAssets = {m_playerCar->tag, m_playerCar->id, m_track->tag, m_track->name};

    // Set up the cameras
    m_freeCamera = FreeCamera(m_track->trackBlocks[0].center, m_window);
    m_hermiteCamera = HermiteCamera(m_track->centerSpline, m_track->trackBlocks[0].center, m_window);
    m_carCamera = CarCamera(m_playerCar->carBodyModel.position, m_window);

    // Generate the collision meshes
    m_physicsEngine.RegisterTrack(m_track);
    m_physicsEngine.RegisterVehicle(m_playerCar);

    if (m_track->tag == NFS_2_SE || m_track->tag == NFS_2 || m_track->tag == NFS_3_PS1)
    {
        m_userParams.useNbData = false;
    }

    // Reset player character to start
    CarAgent::resetToVroad(0, 0, 0.25f, m_track, m_playerCar);
    this->_SpawnRacers(Config::get().nRacers);
}

void RaceSession::_UpdateCameras(float deltaTime)
{
    m_hermiteCamera.UseSpline(m_totalTime);
    // Compute MVP from keyboard and mouse, centered around a target car
    m_carCamera.FollowCar(m_playerCar, m_userParams.windowActive);
    // Compute the MVP matrix from keyboard and mouse input
    m_freeCamera.ComputeMatricesFromInputs(m_userParams.windowActive, deltaTime);
}

Camera RaceSession::_GetCamera()
{
    if (m_userParams.attachCamToHermite) {
        m_activeCameraMode = CameraMode::HERMITE_FLYTHROUGH;
        return m_hermiteCamera;
    } else if (m_userParams.attachCamToCar) {
        m_activeCameraMode = CameraMode::FOLLOW_CAR;
        return m_carCamera;
    } else {
        m_activeCameraMode = CameraMode::FREE_LOOK;
        return m_freeCamera;
    }
}

AssetData RaceSession::Simulate() {
    while (!glfwWindowShouldClose(m_window))
    {
        // glfwGetTime is called only once, the first time this function is called
        static double lastTime = glfwGetTime();
        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        // Update time between engine ticks
        auto deltaTime = float(currentTime - lastTime); // Keep track of time between engine ticks

        if (m_userParams.simulateCars)
        {
            for (auto &racer : m_racerCars)
            {
                racer.simulate();
            }
        }

        // Update Cameras
        this->_UpdateCameras(deltaTime);
        // Set the active camera dependent upon user input
        Camera activeCamera = this->_GetCamera();

        // Step the physics simulation and update physics debug view matrices
        m_physicsEngine.debugDrawer.SetMatrices(activeCamera.viewMatrix, activeCamera.projectionMatrix);
        //m_physicsEngine.StepSimulation(deltaTime);

        bool assetChange = m_renderer.Render(m_totalTime, activeCamera, m_userParams, m_loadedAssets, m_playerCar, m_racerCars, m_physicsEngine);

        if (assetChange)
        {
            return m_loadedAssets;
        }

        // For the next frame, the "last time" will be "now"
        lastTime = currentTime;
        // Keep track of total elapsed time too
        m_totalTime += deltaTime;
        ++m_ticks;
    }

    // Just set a flag temporarily to let main know that we outta here
    m_loadedAssets.trackTag = UNKNOWN;
    return m_loadedAssets;
}

void RaceSession::_SpawnRacers(uint8_t nRacers) {
    float racerSpawnOffset = -0.25f;
    for (uint8_t racerIdx = 0; racerIdx < nRacers; ++racerIdx)
    {
        CarAgent racer(RACER_NAMES[racerIdx % 23], BEST_NETWORK_PATH, m_playerCar, m_track);
        m_physicsEngine.RegisterVehicle(racer.car);
        CarAgent::resetToVroad(0, racerIdx + 1, racerSpawnOffset, m_track, racer.car);
        m_racerCars.emplace_back(racer);
        racerSpawnOffset = -racerSpawnOffset;
    }
}

