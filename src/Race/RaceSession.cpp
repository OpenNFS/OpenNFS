#include "RaceSession.h"

namespace OpenNFS {
    RaceSession::RaceSession(std::shared_ptr<GLFWwindow> const &window, std::shared_ptr<Logger> const &onfsLogger,
                             std::vector<NfsAssetList> const &installedNFS, std::shared_ptr<Track> const &currentTrack,
                             std::shared_ptr<Car> const &currentCar, AssetData &assetData)
        : m_track(currentTrack), m_playerAgent(std::make_shared<PlayerAgent>(m_inputManager, currentCar, currentTrack)),
          m_freeCamera(m_inputManager, m_track->trackBlocks[0].position), m_hermiteCamera(m_track->centerSpline, m_inputManager),
          m_carCamera(m_inputManager), m_physicsManager(m_track),
          m_renderer(window, onfsLogger, installedNFS, m_track, m_physicsManager.debugDrawer),
          m_racerManager(m_playerAgent, m_physicsManager, m_track), m_inputManager(window), m_loadedAssets(assetData) {
    }

    void RaceSession::_UpdateCameras(float const deltaTime) {
        if (m_inputManager.GetWindowStatus() == GAME) {
            switch (m_activeCameraMode) {
            case FOLLOW_CAR:
                // Compute MVP from keyboard and mouse, centered around a target car
                m_carCamera.FollowCar(m_racerManager.racers.at(m_renderer.GetCameraTargetVehicleID())->vehicle, deltaTime);
                break;
            case HERMITE_FLYTHROUGH:
                m_hermiteCamera.UseSpline(m_totalTime);
                break;
            case FREE_LOOK:
                // Compute the MVP matrix from keyboard and mouse input
                m_freeCamera.ComputeMatricesFromInputs(deltaTime);
                break;
            }
        }
    }

    BaseCamera &RaceSession::_GetActiveCamera() {
        if (m_userParams.attachCamToHermite) {
            m_activeCameraMode = HERMITE_FLYTHROUGH;
            return m_hermiteCamera;
        }
        if (m_userParams.attachCamToCar) {
            m_activeCameraMode = FOLLOW_CAR;
            return m_carCamera;
        }
        m_activeCameraMode = FREE_LOOK;
        return m_freeCamera;
    }

    void RaceSession::Update(float const deltaTime) {
        // Clear the screen for next input and grab focus
        m_inputManager.Scan();

        // Update Cameras
        this->_UpdateCameras(deltaTime);

        // Set the active camera dependent upon user input and update Frustum
        auto &activeCamera{this->_GetActiveCamera()};
        activeCamera.UpdateFrustum();

        if (m_userParams.simulateCars) {
            m_racerManager.Simulate();
        }

        m_orbitalManager.Update(activeCamera, m_userParams.timeScaleFactor);

        if (m_inputManager.mouseLeft && m_inputManager.GetWindowStatus() == GAME) {
            std::optional targetedEntity{m_physicsManager.CheckForPicking(m_inputManager.cursorX, m_inputManager.cursorY,
                                                                          activeCamera.viewMatrix, activeCamera.projectionMatrix)};
            // Make the targeted entity 'sticky', else it vanishes after 1 frame
            if (targetedEntity.has_value()) {
                m_targetedEntity = targetedEntity;
            }
        }

        // Step the physics simulation
        m_physicsManager.StepSimulation(deltaTime, m_racerManager.GetRacerResidentTrackblocks());

        if (m_userParams.physicsDebugView) {
            m_physicsManager.GetDynamicsWorld()->debugDrawWorld();
        }

        m_assetChanged = m_renderer.Render(m_totalTime, deltaTime, activeCamera, m_hermiteCamera, m_orbitalManager.GetActiveGlobalLight(),
                                           m_userParams, m_loadedAssets, m_racerManager.racers, m_targetedEntity);

        // Draw the Game UI
        m_uiManager.Update(m_inputManager);

        // Keep track of total elapsed time
        m_totalTime += deltaTime;
        ++m_ticks;
    }

    bool RaceSession::IsFinished() const {
        return false;
    }

    bool RaceSession::AssetChanged() const {
        return m_assetChanged;
    }
} // namespace OpenNFS
