#include "ShadowMapRenderer.h"

#include "../Physics/Car.h"

namespace OpenNFS {
    ShadowMapRenderer::ShadowMapRenderer() {
        // Configure depth map FBO
        glGenFramebuffers(1, &m_depthMapFbo);

        // Create depth texture array for cascades
        glGenTextures(1, &m_depthTextureArrayID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_depthTextureArrayID);

        // Allocate storage for all cascade layers
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT, CSM_NUM_CASCADES, 0, GL_DEPTH_COMPONENT,
                     GL_FLOAT, nullptr);
        // glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT, CSM_NUM_CASCADES);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // Set border colour to 1.0 (no shadow outside map)
        float borderColour[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColour);

        // Enable hardware shadow comparison
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        // Attach first layer initially (change per cascade)
        glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFbo);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTextureArrayID, 0, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        // Check that the framebuffer is ok
        CHECK_F(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "CSM Depth FBO is not complete");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifndef __APPLE__
        // Gahh, this won't work on a Mac, we can't request more than a GL 4.1 Context
        glGenTextures(CSM_NUM_CASCADES, m_depthTextureViews.data());
        for (int32_t i = 0; i < CSM_NUM_CASCADES; ++i) {
            glTextureView(m_depthTextureViews[i], GL_TEXTURE_2D, m_depthTextureArrayID, GL_DEPTH_COMPONENT32F, 0, 1, i, 1);
            glTextureParameteri(m_depthTextureViews[i], GL_TEXTURE_COMPARE_MODE, GL_NONE);
            glTextureParameteri(m_depthTextureViews[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_depthTextureViews[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
#endif
    }

    void ShadowMapRenderer::RenderCascade(uint32_t const cascadeIndex, glm::mat4 const &lightSpaceMatrix, GLuint const trackTextureArrayID,
                                          std::vector<std::shared_ptr<Entity>> const &visibleEntities,
                                          std::vector<std::shared_ptr<CarAgent>> const &racers) {
        // Attach the correct layer of the texture array
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTextureArrayID, 0, cascadeIndex);

        glClear(GL_DEPTH_BUFFER_BIT);

        m_depthShader.loadLightSpaceMatrix(lightSpaceMatrix);

        // Render track entities
        m_depthShader.bindTextureArray(trackTextureArrayID);
        for (auto &entity : visibleEntities) {
            m_depthShader.loadTransformMatrix(entity->ModelMatrix);
            entity->Render();
        }

        // Render cars
        for (auto &racer : racers) {
            if (racer->vehicle->renderInfo.isMultitexturedModel) {
                m_depthShader.bindTextureArray(racer->vehicle->renderInfo.textureArrayID);
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
    }

    void ShadowMapRenderer::Render(GlobalLight const *light, BaseCamera const &camera, GLuint const trackTextureArrayID,
                                   std::vector<std::shared_ptr<Entity>> const &visibleEntities,
                                   std::vector<std::shared_ptr<CarAgent>> const &racers) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        m_depthShader.use();

        glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFbo);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        // Render each cascade
        for (uint32_t i = 0; i < CSM_NUM_CASCADES; ++i) {
            RenderCascade(i, light->cascadeData.lightSpaceMatrices[i], trackTextureArrayID, visibleEntities, racers);
        }

        // Restore viewport
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_depthShader.unbind();
        m_depthShader.HotReload();
    }

    GLuint ShadowMapRenderer::GetTextureArrayID() const {
        return m_depthTextureArrayID;
    }

    GLuint ShadowMapRenderer::GetTextureViewID(uint8_t layer) const {
        return m_depthTextureViews.at(layer);
    }

    ShadowMapRenderer::~ShadowMapRenderer() {
        glDeleteFramebuffers(1, &m_depthMapFbo);
        glDeleteTextures(1, &m_depthTextureArrayID);
        m_depthShader.cleanup();
    }

} // namespace OpenNFS
