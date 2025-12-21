#include "ShadowMapRenderer.h"

#include "../Physics/Car.h"

namespace OpenNFS {
    ShadowMapRenderer::ShadowMapRenderer() {
        // Configure depth map FBO
        glGenFramebuffers(1, &m_depthMapFbo);
        // Create depth texture
        glGenTextures(1, &m_depthTextureID);
        glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureID, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        // Always check that our framebuffer is ok
        CHECK_F(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Depth FBO is nae good");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ShadowMapRenderer::Render(float nearPlane,
                                   float farPlane,
                                   GlobalLight const *light,
                                   GLuint const trackTextureArrayID,
                                   std::vector<std::shared_ptr<Entity>> const &visibleEntities,
                                   std::vector<std::shared_ptr<CarAgent>> const &racers) {
        /* ------- SHADOW MAPPING ------- */
        m_depthShader.use();
        m_depthShader.loadLightSpaceMatrix(light->lightSpaceMatrix);
        m_depthShader.bindTextureArray(trackTextureArrayID);

        glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFbo);
        glClear(GL_DEPTH_BUFFER_BIT);

        /* Render the track using this simple shader to get depth texture to test against during draw */
        for (auto &entity : visibleEntities) {
            m_depthShader.loadTransformMatrix(entity->ModelMatrix);
            entity->Render();
        }

        /* And the Cars */
        for (auto &racer : racers) {
            if (racer->vehicle->renderInfo.isMultitexturedModel) {
                m_depthShader.bindTextureArray(racer->vehicle->renderInfo.textureArrayID);
            } else {
                // m_depthShader.loadCarTexture(racer->vehicle->renderInfo.textureID);
            }
            for (auto &misc_model : racer->vehicle->miscModels) {
                m_depthShader.loadTransformMatrix(misc_model.ModelMatrix);
                misc_model.Render();
            }
            m_depthShader.loadTransformMatrix(racer->vehicle->leftFrontWheelModel.ModelMatrix);
            racer->vehicle->leftFrontWheelModel.Render();
            m_depthShader.loadTransformMatrix(racer->vehicle->leftRearWheelModel.ModelMatrix);
            racer->vehicle->leftRearWheelModel.Render();
            m_depthShader.loadTransformMatrix(racer->vehicle->rightFrontWheelModel.ModelMatrix);
            racer->vehicle->rightFrontWheelModel.Render();
            m_depthShader.loadTransformMatrix(racer->vehicle->rightRearWheelModel.ModelMatrix);
            racer->vehicle->rightRearWheelModel.Render();
            m_depthShader.loadTransformMatrix(racer->vehicle->carBodyModel.ModelMatrix);
            racer->vehicle->carBodyModel.Render();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_depthShader.unbind();
        m_depthShader.HotReload();
    }

    GLuint ShadowMapRenderer::GetTextureID() const {
        return m_depthTextureID;
    }

    ShadowMapRenderer::~ShadowMapRenderer() {
        glDeleteFramebuffers(1, &m_depthMapFbo);
        m_depthShader.cleanup();
    }

} // namespace OpenNFS
