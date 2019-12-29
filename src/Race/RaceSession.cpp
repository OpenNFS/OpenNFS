#include "RaceSession.h"

#include <imgui.h>

RaceSession::RaceSession(GLFWwindow *glWindow,
                         std::shared_ptr<Logger> &onfsLogger,
                         const std::vector<NfsAssetList> &installedNFS,
                         std::shared_ptr<ONFSTrack> currentTrack,
                         std::shared_ptr<Car> &currentCar) :
        m_pWindow(glWindow), m_track(currentTrack), m_playerCar(currentCar),
        m_renderer(glWindow, onfsLogger, installedNFS, m_track, std::make_shared<BulletDebugDrawer>(m_physicsEngine.debugDrawer))
{

    m_loadedAssets = {m_playerCar->tag, m_playerCar->id, m_track->tag, m_track->name};

    // Set up the cameras
    m_freeCamera = FreeCamera(m_track->trackBlocks[0].center, m_pWindow);
    m_hermiteCamera = HermiteCamera(m_track->centerSpline, m_track->trackBlocks[0].center, m_pWindow);
    m_carCamera = CarCamera(m_playerCar->carBodyModel.position, m_pWindow);

    // Generate the collision meshes
    m_physicsEngine.RegisterTrack(m_track);
    //m_physicsEngine.RegisterVehicle(m_playerCar);

    // No neighbour data for anything except NFS3
    m_userParams.useNbData = !(m_track->tag == NFS_2_SE || m_track->tag == NFS_2 || m_track->tag == NFS_3_PS1);

    // Reset player character to start
    //CarAgent::resetToVroad(0, 0, 0.25f, m_track, m_playerCar);
    //this->_SpawnRacers(Config::get().nRacers);
}

void RaceSession::_UpdateCameras(float deltaTime)
{
    m_hermiteCamera.UseSpline(m_totalTime);

    if (m_windowStatus == WindowStatus::GAME)
    {
        // Compute MVP from keyboard and mouse, centered around a target car
        m_carCamera.FollowCar(m_playerCar);
        // Compute the MVP matrix from keyboard and mouse input
        m_freeCamera.ComputeMatricesFromInputs(deltaTime);
    }
}

Camera RaceSession::_GetCamera()
{
    if (m_userParams.attachCamToHermite)
    {
        m_activeCameraMode = CameraMode::HERMITE_FLYTHROUGH;
        return m_hermiteCamera;
    }
    else if (m_userParams.attachCamToCar)
    {
        m_activeCameraMode = CameraMode::FOLLOW_CAR;
        return m_carCamera;
    }
    else
    {
        m_activeCameraMode = CameraMode::FREE_LOOK;
        return m_freeCamera;
    }
}

AssetData RaceSession::Simulate()
{
    while (!glfwWindowShouldClose(m_pWindow))
    {
        // glfwGetTime is called only once, the first time this function is called
        static double lastTime = glfwGetTime();
        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        // Update time between engine ticks
        auto deltaTime = float(currentTime - lastTime); // Keep track of time between engine ticks

        this->_GetInputsAndClear();

        // Update Cameras
        this->_UpdateCameras(deltaTime);
        // Set the active camera dependent upon user input
        Camera activeCamera = this->_GetCamera();

        if (m_userParams.simulateCars)
        {
            this->_SimulateRacers();
        }

        // Step the physics simulation
        m_physicsEngine.StepSimulation(deltaTime);
        if (m_userParams.physicsDebugView)
        { m_physicsEngine.GetDynamicsWorld()->debugDrawWorld(); }

        bool assetChange = m_renderer.Render(m_totalTime, activeCamera, m_hermiteCamera, m_userParams, m_loadedAssets, m_playerCar, m_racerCars);

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

void RaceSession::_SpawnRacers(uint8_t nRacers)
{
    float racerSpawnOffset = -0.25f;
    for (uint8_t racerIdx = 0; racerIdx < nRacers; ++racerIdx)
    {
        CarAgent racer(racerIdx % 23, BEST_NETWORK_PATH, m_playerCar, m_track);
        m_physicsEngine.RegisterVehicle(racer.car);
        CarAgent::resetToVroad(0, racerIdx + 1, racerSpawnOffset, m_track, racer.car);
        m_racerCars.emplace_back(racer);
        racerSpawnOffset = -racerSpawnOffset;
    }
}

void RaceSession::_SimulateRacers()
{
    for (auto &racer : m_racerCars)
    {
        racer.simulate();
    }
}

void RaceSession::_GetInputsAndClear()
{
    glClearColor(0.1f, 0.f, 0.5f, 1.f);
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwPollEvents();

    // Detect a click on the 3D Window by detecting a click that isn't on ImGui
    if ((glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) && (!ImGui::GetIO().WantCaptureMouse))
    {
        m_windowStatus = WindowStatus::GAME;
        ImGui::GetIO().MouseDrawCursor = false;
    }
    else if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        m_windowStatus = WindowStatus::UI;
        ImGui::GetIO().MouseDrawCursor = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

