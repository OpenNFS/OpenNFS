#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Camera/CarCamera.h"
#include "../Camera/FreeCamera.h"
#include "../Camera/HermiteCamera.h"
#include "../Config.h"
#include "../Input/InputManager.h"
#include "../Physics/PhysicsManager.h"
#include "../Renderer/Renderer.h"
#include "../Scene/Track.h"
#include "../Util/Logger.h"
#include "Agents/PlayerAgent.h"
#include "OrbitalManager.h"
#include "RacerManager.h"

namespace OpenNFS {
    class RaceSession {
      public:
        RaceSession(std::shared_ptr<GLFWwindow> const &window, std::shared_ptr<Logger> const &onfsLogger,
                    std::vector<NfsAssetList> const &installedNFS, std::shared_ptr<Track> const &currentTrack,
                    std::shared_ptr<Car> const &currentCar, AssetData &assetData);

        // New state-friendly interface
        void Update(float deltaTime);
        [[nodiscard]] bool IsFinished() const;
        [[nodiscard]] bool AssetChanged() const;
        [[nodiscard]] AssetData const &GetLoadedAssets() const {
            return m_loadedAssets;
        }

      private:
        BaseCamera &_GetActiveCamera();
        void _UpdateCameras(float deltaTime);

        AssetData &m_loadedAssets;
        CameraMode m_activeCameraMode{CameraMode::FREE_LOOK};
        std::optional<Entity *> m_targetedEntity;

        std::shared_ptr<Track> m_track;
        std::shared_ptr<PlayerAgent> m_playerAgent;
        FreeCamera m_freeCamera;
        HermiteCamera m_hermiteCamera;
        CarCamera m_carCamera;

        PhysicsManager m_physicsManager;
        Renderer m_renderer;
        RacerManager m_racerManager;
        OrbitalManager m_orbitalManager;
        InputManager m_inputManager;
        UIManager m_uiManager;

        ParamData m_userParams;
        uint64_t m_ticks{0}; // Engine ticks elapsed
        float m_totalTime{0};
        bool m_assetChanged{false};
    };
} // namespace OpenNFS
