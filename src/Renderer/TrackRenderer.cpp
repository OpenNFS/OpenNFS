#include "TrackRenderer.h"

#include "../Physics/Car.h"
#include "../Scene/GlobalLight.h"

namespace OpenNFS {
    TrackRenderer::TrackRenderer() {
        InitLightQuadBuffers();
    }

    void TrackRenderer::InitLightQuadBuffers() {
        // Billboard quad vertices: position (vec3) + UV (vec2) interleaved
        constexpr float kLightSize = 1.0f;

        // 6 vertices for 2 triangles (indices: 0,1,2 and 0,2,3)
        GLfloat const quadVertices[] = {
            // Position                     // UV
            -kLightSize, -kLightSize, 0.0f, 1.0f, 1.0f, // bottom-left
            -kLightSize, kLightSize,  0.0f, 0.0f, 1.0f, // top-left
            kLightSize,  kLightSize,  0.0f, 0.0f, 0.0f, // top-right
            -kLightSize, -kLightSize, 0.0f, 1.0f, 1.0f, // bottom-left
            kLightSize,  kLightSize,  0.0f, 0.0f, 0.0f, // top-right
            kLightSize,  -kLightSize, 0.0f, 1.0f, 0.0f, // bottom-right
        };

        glGenVertexArrays(1, &m_lightQuadVAO);
        glGenBuffers(1, &m_lightQuadVBO);

        glBindVertexArray(m_lightQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_lightQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // Attribute 0: position (vec3)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);

        // Attribute 1: UV (vec2)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

        // Reset state
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

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
        if (racers[0]->vehicle->vehicleState.headlightsActive) {
            for (auto const &racer : racers) {
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
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        m_billboardShader.use();
        m_billboardShader.loadMatrices(camera.projectionMatrix, camera.viewMatrix);

        glBindVertexArray(m_lightQuadVAO);

        for (auto const &light : lights) {
            if (light->type == TRACK_LIGHT) {
                auto const trackLight = dynamic_cast<TrackLight const *>(light);
                m_billboardShader.loadLight(trackLight);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }

        glBindVertexArray(0);
        m_billboardShader.unbind();
        m_billboardShader.HotReload();

        glDepthMask(GL_TRUE);
    }

    TrackRenderer::~TrackRenderer() {
        // Cleanup VAO/VBO
        if (m_lightQuadVAO != 0) {
            glDeleteVertexArrays(1, &m_lightQuadVAO);
        }
        if (m_lightQuadVBO != 0) {
            glDeleteBuffers(1, &m_lightQuadVBO);
        }

        // Cleanup shaders
        m_trackShader.cleanup();
        m_billboardShader.cleanup();
    }
} // namespace OpenNFS
