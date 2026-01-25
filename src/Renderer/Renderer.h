#pragma once

#include <backends/imgui_impl_glfw.h>
#include <optional>

#include "../Camera/HermiteCamera.h"
#include "../Config.h"
#include "../Race/Agents/CarAgent.h"
#include "../Scene/Entity.h"
#include "../Scene/GlobalLight.h"
#include "../Scene/Track.h"
#include "../UI/UIManager.h"
#include "../Util/Logger.h"

#include "CarRenderer.h"
#include "DebugRenderer.h"
#include "MiniMapRenderer.h"
#include "ShadowMapRenderer.h"
#include "SkidMarkRenderer.h"
#include "SkyRenderer.h"
#include "TrackRenderer.h"

namespace OpenNFS {
    // Forward declarations
    struct VehicleTrails;
    class SkidMarkSystem;

    struct VisibleSet {
        std::vector<std::shared_ptr<Entity>> entities;
        std::vector<BaseLight const *> lights;
    };

    class Renderer {
      public:
        Renderer(std::shared_ptr<GLFWwindow> const &window, std::shared_ptr<Logger> const &onfsLogger,
                 std::vector<NfsAssetList> installedNFS, std::shared_ptr<Track> const &currentTrack,
                 std::shared_ptr<BulletDebugDrawer> const &debugDrawer);

        static void GlfwError(int const id, char const *description) {
            LOG(WARNING) << description << " id: " << id;
        }

        static void WindowSizeCallback(GLFWwindow *window, int const width, int const height) {
            Config::get().windowSizeX = width;
            Config::get().windowSizeY = height;
        }

        static std::shared_ptr<GLFWwindow> InitOpenGL(std::string const &windowName);
        static void Shutdown();
        static void _DrawMetadata(Entity const *targetEntity);
        bool Render(float totalTime, float deltaTime, BaseCamera const &activeCamera, HermiteCamera const &hermiteCamera,
                    GlobalLight const *activeLight, ParamData &userParams, AssetData &loadedAssets,
                    std::vector<std::shared_ptr<CarAgent>> const &racers, std::optional<Entity *> targetedEntity,
                    SkidMarkSystem const *skidMarkSystem = nullptr);
        [[nodiscard]] uint32_t GetCameraTargetVehicleID() const;
        static void NewFrame();
        static void EndFrame();

      private:
        bool _DrawMenuBar(AssetData &loadedAssets) const;
        void _DrawDebugUI(ParamData &userParams, float deltaTime, BaseCamera const &camera);
        void _DrawNFS4PhysicsDebug(std::vector<std::shared_ptr<CarAgent>> const &racers);
        static std::vector<uint32_t> _GetLocalTrackBlockIDs(std::shared_ptr<Track> const &track, BaseCamera const &camera);
        static VisibleSet _FrustumCull(std::shared_ptr<Track> const &track, BaseCamera const &camera, GlobalLight const *globalLight,
                                       std::shared_ptr<CarAgent> const &racer, ParamData const &userParams);

        std::shared_ptr<GLFWwindow> m_window;
        std::shared_ptr<Logger> m_logger;
        std::vector<NfsAssetList> m_nfsAssetList;
        std::shared_ptr<Track> m_track;
        uint32_t m_cameraTargetVehicleID{0};
        uint32_t m_dbgVizShadowMapCascadeID{0};
        uint32_t m_numRacers{0};

        TrackRenderer m_trackRenderer;
        CarRenderer m_carRenderer;
        SkyRenderer m_skyRenderer;
        ShadowMapRenderer m_shadowMapRenderer;
        DebugRenderer m_debugRenderer;
        MiniMapRenderer m_miniMapRenderer;
        SkidMarkRenderer m_skidMarkRenderer;

        // Smoothed deltatime for stable FPS display
        static constexpr size_t kDeltaTimeHistorySize = 60; // Average over 60 frames
        std::array<float, kDeltaTimeHistorySize> m_deltaTimeHistory{};
        size_t m_deltaTimeHistoryIndex{0};
    };
} // namespace OpenNFS
