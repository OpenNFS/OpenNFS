#pragma once

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

#include "../Camera/HermiteCamera.h"
#include "../Scene/Entity.h"
#include "../Scene/GlobalLight.h"
#include "../Scene/Track.h"
#include "../RaceNet/Agents/CarAgent.h"
#include "../Util/Logger.h"
#include "../Config.h"

#include "CarRenderer.h"
#include "TrackRenderer.h"
#include "SkyRenderer.h"
#include "ShadowMapRenderer.h"
#include "DebugRenderer.h"
#include "MenuRenderer.h"

namespace OpenNFS {
    struct VisibleSet {
        std::vector<std::shared_ptr<Entity> > entities;
        std::vector<std::shared_ptr<BaseLight> > lights;
    };

    class Renderer {
    public:
        Renderer(const std::shared_ptr<GLFWwindow> &window,
                 const std::shared_ptr<Logger> &onfsLogger,
                 const std::vector<NfsAssetList> &installedNFS,
                 const std::shared_ptr<Track> &currentTrack,
                 const std::shared_ptr<BulletDebugDrawer> &debugDrawer);

        ~Renderer();

        static void GlfwError(int id, const char *description) {
            LOG(WARNING) << description;
        }

        static void WindowSizeCallback(GLFWwindow *window, int width, int height) {
            Config::get().windowSizeX = width;
            Config::get().windowSizeY = height;
        }

        static std::shared_ptr<GLFWwindow> InitOpenGL(uint32_t resolutionX, uint32_t resolutionY,
                                                      const std::string &windowName);

        static void _DrawMetadata(Entity *targetEntity);

        bool Render(float totalTime,
                    const BaseCamera &activeCamera,
                    const HermiteCamera &hermiteCamera,
                    const std::shared_ptr<GlobalLight> &activeLight,
                    ParamData &userParams,
                    AssetData &loadedAssets,
                    const std::vector<std::shared_ptr<CarAgent> > &racers,
                    std::optional<Entity *>
                    targetedEntity);

    private:
        void _InitialiseIMGUI() const;

        bool _DrawMenuBar(AssetData &loadedAssets) const;

        void _DrawDebugUI(ParamData &userParams, const BaseCamera &camera) const;

        static std::vector<uint32_t> _GetLocalTrackBlockIDs(const std::shared_ptr<Track> &track,
                                                            const BaseCamera &camera, ParamData const &userParams);

        static VisibleSet _FrustumCull(const std::shared_ptr<Track> &track, const BaseCamera &camera,
                                       ParamData const &userParams);

        std::shared_ptr<GLFWwindow> m_window;
        std::shared_ptr<Logger> m_logger;
        std::vector<NfsAssetList> m_nfsAssetList;
        std::shared_ptr<Track> m_track;

        TrackRenderer m_trackRenderer;
        CarRenderer m_carRenderer;
        SkyRenderer m_skyRenderer;
        ShadowMapRenderer m_shadowMapRenderer;
        DebugRenderer m_debugRenderer;
        MenuRenderer m_menuRenderer;
    };
} // namespace OpenNFS
