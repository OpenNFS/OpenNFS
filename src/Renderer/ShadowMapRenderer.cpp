#include "ShadowMapRenderer.h"

ShadowMapRenderer::ShadowMapRenderer() {
    // Configure depth map FBO
    glGenFramebuffers(1, &m_fboDepthMap);
    // Create depth texture
    glGenTextures(1, &m_depthTextureID);
    glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboDepthMap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Always check that our framebuffer is ok
    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Depth FBO is nae good");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapRenderer::Render(float nearPlane,
                               float farPlane,
                               const std::shared_ptr<GlobalLight> &light,
                               GLuint trackTextureArrayID,
                               const std::vector<std::shared_ptr<Entity>> &visibleEntities,
                               const std::vector<std::shared_ptr<CarAgent>> &racers) {
    /* ------- SHADOW MAPPING ------- */
    m_depthShader.use();
    m_depthShader.loadLightSpaceMatrix(light->lightSpaceMatrix);
    m_depthShader.bindTextureArray(trackTextureArrayID);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboDepthMap);
    glClear(GL_DEPTH_BUFFER_BIT);

    /* Render the track using this simple shader to get depth texture to test against during draw */
    for (auto &entity : visibleEntities) {
        m_depthShader.loadTransformMatrix(std::get<TrackModel>(entity->raw).ModelMatrix);
        std::get<TrackModel>(entity->raw).render();
    }

    /* And the Cars */
    for (auto &racer : racers) {
        m_depthShader.bindTextureArray(racer->vehicle->renderInfo.textureArrayID);
        for (auto &misc_model : racer->vehicle->miscModels) {
            m_depthShader.loadTransformMatrix(misc_model.ModelMatrix);
            misc_model.render();
        }
        m_depthShader.loadTransformMatrix(racer->vehicle->leftFrontWheelModel.ModelMatrix);
        racer->vehicle->leftFrontWheelModel.render();
        m_depthShader.loadTransformMatrix(racer->vehicle->leftRearWheelModel.ModelMatrix);
        racer->vehicle->leftRearWheelModel.render();
        m_depthShader.loadTransformMatrix(racer->vehicle->rightFrontWheelModel.ModelMatrix);
        racer->vehicle->rightFrontWheelModel.render();
        m_depthShader.loadTransformMatrix(racer->vehicle->rightRearWheelModel.ModelMatrix);
        racer->vehicle->rightRearWheelModel.render();
        m_depthShader.loadTransformMatrix(racer->vehicle->carBodyModel.ModelMatrix);
        racer->vehicle->carBodyModel.render();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Config::get().resX, Config::get().resY);
    m_depthShader.unbind();
    m_depthShader.HotReload();
}

ShadowMapRenderer::~ShadowMapRenderer() {
    glDeleteFramebuffers(1, &m_fboDepthMap);
    m_depthShader.cleanup();
};
