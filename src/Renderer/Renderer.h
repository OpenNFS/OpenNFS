#pragma once

#include <backends/imgui_impl_glfw.h>
#include <optional>

#include "../Camera/HermiteCamera.h"
#include "../Config.h"
#include "../Race/Agents/CarAgent.h"
#include "../Scene/Entity.h"
#include "../Scene/GlobalLight.h"
#include "../Scene/Track.h"
#include "../Util/Logger.h"
#include "../UI/UIManager.h"

#include "CarRenderer.h"
#include "DebugRenderer.h"
#include "MiniMapRenderer.h"
#include "ShadowMapRenderer.h"
#include "SkyRenderer.h"
#include "TrackRenderer.h"

namespace OpenNFS {
    struct VisibleSet {
        std::vector<std::shared_ptr<Entity>> entities;
        std::vector<BaseLight const *> lights;
    };

    class Renderer {
      public:
        Renderer(std::shared_ptr<GLFWwindow> const &window,
                 std::shared_ptr<Logger> const &onfsLogger,
                 std::vector<NfsAssetList> const &installedNFS,
                 Track const &currentTrack,
                 std::shared_ptr<BulletDebugDrawer> const &debugDrawer);
        ~Renderer();

        static void GlfwError(int id, char const *description) {
            LOG(WARNING) << description;
        }

        static void WindowSizeCallback(GLFWwindow *window, int width, int height) {
            Config::get().windowSizeX = width;
            Config::get().windowSizeY = height;
        }

        static std::shared_ptr<GLFWwindow> InitOpenGL(uint32_t resolutionX, uint32_t resolutionY, std::string const &windowName);
        static void _DrawMetadata(Entity const *targetEntity);
        bool Render(float totalTime,
                    BaseCamera const &activeCamera,
                    HermiteCamera const &hermiteCamera,
                    GlobalLight const *activeLight,
                    ParamData &userParams,
                    AssetData &loadedAssets,
                    std::vector<std::shared_ptr<CarAgent>> const &racers,
                    std::optional<Entity *> targetedEntity);
        [[nodiscard]] uint32_t GetCameraTargetVehicleID() const;
        void NewFrame();

      private:
        void _InitialiseIMGUI() const;
        bool _DrawMenuBar(AssetData &loadedAssets) const;
        void _DrawDebugUI(ParamData &userParams, BaseCamera const &camera);
        static std::vector<uint32_t> _GetLocalTrackBlockIDs(Track const &track, BaseCamera const &camera);
        static VisibleSet _FrustumCull(Track const &track,
                                       BaseCamera const &camera,
                                       GlobalLight const *globalLight,
                                       ParamData const &userParams);

        std::shared_ptr<GLFWwindow> m_window;
        std::shared_ptr<Logger> m_logger;
        std::vector<NfsAssetList> m_nfsAssetList;
        Track const &m_track;
        uint32_t m_cameraTargetVehicleID{0};
        uint32_t m_numRacers{0};

        TrackRenderer m_trackRenderer;
        CarRenderer m_carRenderer;
        SkyRenderer m_skyRenderer;
        ShadowMapRenderer m_shadowMapRenderer;
        DebugRenderer m_debugRenderer;
        MiniMapRenderer m_miniMapRenderer;
    };
} // namespace OpenNFS
