#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include "../Camera/FreeCamera.h"
#include "../Camera/HermiteCamera.h"
#include "../Camera/CarCamera.h"
#include "../Scene/Entity.h"
#include "../Scene/Lights/GlobalLight.h"
#include "../Loaders/TrackLoader.h"
#include "../Physics/PhysicsEngine.h"
#include "../RaceNet/Agents/CarAgent.h"
#include "../Util/Logger.h"
#include "../Config.h"

#include "HermiteCurve.h"
#include "CarRenderer.h"
#include "TrackRenderer.h"
#include "SkyRenderer.h"
#include "ShadowMapRenderer.h"
#include "DebugRenderer.h"
/*#include "MenuRenderer.h"*/

struct VisibleSet
{
    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<std::shared_ptr<BaseLight>> lights;
};

class Renderer {
public:
    Renderer(GLFWwindow *pWindow, std::shared_ptr<Logger> &onfsLogger, const std::vector<NfsAssetList> &installedNFS, std::shared_ptr<ONFSTrack> currentTrack, const std::shared_ptr<BulletDebugDrawer> &debugDrawer);
    ~Renderer();
    static void GlfwError(int id, const char *description) {
        LOG(WARNING) << description;
    }
    static void WindowSizeCallback(GLFWwindow *window, int width, int height) {
        Config::get().resX = width;
        Config::get().resY = height;
    }
    static GLFWwindow *InitOpenGL(int resolutionX, int resolutionY, const std::string &windowName);
    bool Render(float totalTime,
                const std::shared_ptr<BaseCamera> &activeCamera,
                const std::shared_ptr<HermiteCamera> &hermiteCamera,
                const std::shared_ptr<GlobalLight> &activeLight,
                ParamData &userParams,
                AssetData &loadedAssets,
                const std::vector<std::shared_ptr<CarAgent>> &racers);

private:
    void _InitialiseIMGUI();
    static void _DrawMetadata(Entity *targetEntity);
    bool _DrawMenuBar(AssetData &loadedAssets);
    void _DrawUI(ParamData &userParams, const std::shared_ptr<BaseCamera> &camera);
    static std::vector<uint32_t> _GetLocalTrackBlockIDs(const shared_ptr<ONFSTrack> &track, const std::shared_ptr<BaseCamera> &camera, ParamData &userParams);
    static VisibleSet _FrustumCull(const std::shared_ptr<ONFSTrack> &track, const std::shared_ptr<BaseCamera> &camera, ParamData &userParams);

    GLFWwindow *m_pWindow;
    std::shared_ptr<Logger> m_logger;
    std::vector<NfsAssetList> m_nfsAssetList;
    std::shared_ptr<ONFSTrack> m_track;

    TrackRenderer m_trackRenderer;
    CarRenderer m_carRenderer;
    SkyRenderer m_skyRenderer;
    ShadowMapRenderer m_shadowMapRenderer;
    DebugRenderer m_debugRenderer;
    /*MenuRenderer menuRenderer;*/

    /* State */
    bool m_entityTargeted = false;
    Entity *m_pTargetedEntity = nullptr;
};
