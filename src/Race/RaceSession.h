#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Agents/PlayerAgent.h"
#include "../Camera/CarCamera.h"
#include "../Camera/FreeCamera.h"
#include "../Camera/HermiteCamera.h"
#include "../Config.h"
#include "../Input/InputManager.h"
#include "../Physics/PhysicsManager.h"
#include "../Renderer/Renderer.h"
#include "../Scene/Track.h"
#include "../Util/Logger.h"
#include "OrbitalManager.h"
#include "RacerManager.h"

namespace OpenNFS {
    class RaceSession {
    public:
        RaceSession(const std::shared_ptr<GLFWwindow> &window,
                    const std::shared_ptr<Logger> &onfsLogger,
                    const std::vector<NfsAssetList> &installedNFS,
                    const Track &currentTrack,
                    const std::shared_ptr<Car> &currentCar);
        AssetData Simulate();

    private:
        BaseCamera &_GetActiveCamera();
        void _UpdateCameras(float deltaTime);

        AssetData m_loadedAssets;
        CameraMode m_activeCameraMode {CameraMode::FREE_LOOK};
        std::optional<Entity *> m_targetedEntity;

        std::shared_ptr<GLFWwindow> m_window;
        const Track &m_track;
        std::shared_ptr<PlayerAgent> m_playerAgent;
        FreeCamera m_freeCamera;
        HermiteCamera m_hermiteCamera;
        CarCamera m_carCamera;

        PhysicsManager m_physicsManager;
        Renderer m_renderer;
        RacerManager m_racerManager;
        OrbitalManager m_orbitalManager;
        InputManager m_inputManager;

        ParamData m_userParams;
        uint64_t m_ticks {0}; // Engine ticks elapsed
        float m_totalTime {0};
    };
} // namespace OpenNFS
