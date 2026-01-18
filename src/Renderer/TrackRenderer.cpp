#include "TrackRenderer.h"

#include "../Physics/Car.h"
#include "../Scene/GlobalLight.h"

namespace OpenNFS {
    void TrackRenderer::Render(std::vector<std::shared_ptr<CarAgent>> const &racers, BaseCamera const &camera,
                               GLuint const trackTextureArrayID, std::vector<std::shared_ptr<Entity>> const &visibleEntities,
                               std::vector<LibOpenNFS::BaseLight const *> const &lights, ParamData const &userParams,
                               GLuint const depthTextureID, float const ambientFactor) {
        m_trackShader.use();
        // This shader state doesn't change during a track renderpass
        m_trackShader.setClassic(userParams.useClassicGraphics);
        m_trackShader.loadProjectionViewMatrices(camera.projectionMatrix, camera.viewMatrix);
        m_trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
        m_trackShader.bindTextureArray(trackTextureArrayID);
        m_trackShader.loadAmbientFactor(ambientFactor);
        m_trackShader.loadLights(lights);

        // Collect all active spotlights from all racers
        std::vector<Spotlight> activeSpotlights;
        for (auto const &racer : racers) {
            if (racer->vehicle->vehicleState.headlightsActive) {
                activeSpotlights.push_back(racer->vehicle->leftHeadLight);
                activeSpotlights.push_back(racer->vehicle->rightHeadLight);
            }
        }
        m_trackShader.loadSpotlights(activeSpotlights);

        // Global Light is always 0th index
        auto const globalLight{dynamic_cast<GlobalLight const *>(lights.at(0))};
        CHECK_F(globalLight->type == LibOpenNFS::LightType::GLOBAL_LIGHT, "Light 0 is not GLOBAL_LIGHT");

        // Load CSM data for shadows
        m_trackShader.loadCascadeData(globalLight->cascadeData);
        m_trackShader.loadShadowMapTextureArray(depthTextureID);

        // Render the per-trackblock data
        for (auto &entity : visibleEntities) {
            m_trackShader.loadTransformMatrix(entity->ModelMatrix);
            entity->Render();
        }

        m_trackShader.unbind();
        m_trackShader.HotReload();
    }

    void TrackRenderer::RenderLights(BaseCamera const &camera, std::vector<BaseLight const *> const &lights) {
        m_billboardShader.use();

        // for (auto &light : lights) {
        //     if (light->type == TRACK_LIGHT) {
        //         TrackLight const *trackLight {std::static_pointer_cast<const TrackLight>(light)};
        //         m_billboardShader.loadMatrices(camera.projectionMatrix, camera.viewMatrix);
        //         m_billboardShader.loadLight(trackLight);
        //         // trackLight->model.render();
        //     }
        // }
        m_billboardShader.unbind();
        m_billboardShader.HotReload();
    }

    TrackRenderer::~TrackRenderer() {
        // Cleanup VBOs and shaders
        m_trackShader.cleanup();
        m_billboardShader.cleanup();
    }
} // namespace OpenNFS
