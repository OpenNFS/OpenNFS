#include "ShadowMapRenderer.h"

ShadowMapRenderer::ShadowMapRenderer()
{
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
    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Depth FBO is nae good.");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapRenderer::Render(float nearPlane, float farPlane, const GlobalLight &light,  GLuint trackTextureArrayID, const std::vector<std::shared_ptr<Entity>> &visibleEntities, const std::shared_ptr<Car> &car, const std::vector<CarAgent> &racers){
    /* ------- SHADOW MAPPING ------- */
    m_depthShader.use();
    m_depthShader.loadLightSpaceMatrix(light.lightSpaceMatrix);
    m_depthShader.bindTextureArray(trackTextureArrayID);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboDepthMap);
    glClear(GL_DEPTH_BUFFER_BIT);

    /* Render the track using this simple shader to get depth texture to test against during draw */
    for (auto &entity : visibleEntities) {
        m_depthShader.loadTransformMatrix(boost::get<Track>(entity->glMesh).ModelMatrix);
        boost::get<Track>(entity->glMesh).render();
    }

    /* And the Cars */
    m_depthShader.bindTextureArray(car->textureArrayID);
    for (auto &misc_model : car->miscModels) {
        m_depthShader.loadTransformMatrix(misc_model.ModelMatrix);
        misc_model.render();
    }
    m_depthShader.loadTransformMatrix(car->leftFrontWheelModel.ModelMatrix);
    car->leftFrontWheelModel.render();
    m_depthShader.loadTransformMatrix(car->leftRearWheelModel.ModelMatrix);
    car->leftRearWheelModel.render();
    m_depthShader.loadTransformMatrix(car->rightFrontWheelModel.ModelMatrix);
    car->rightFrontWheelModel.render();
    m_depthShader.loadTransformMatrix(car->rightRearWheelModel.ModelMatrix);
    car->rightRearWheelModel.render();
    m_depthShader.loadTransformMatrix(car->carBodyModel.ModelMatrix);
    car->carBodyModel.render();

    for(auto &racer : racers){
        m_depthShader.bindTextureArray(racer.car->textureArrayID);
        for (auto &misc_model : racer.car->miscModels) {
            m_depthShader.loadTransformMatrix(misc_model.ModelMatrix);
            misc_model.render();
        }
        m_depthShader.loadTransformMatrix(racer.car->leftFrontWheelModel.ModelMatrix);
        racer.car->leftFrontWheelModel.render();
        m_depthShader.loadTransformMatrix(racer.car->leftRearWheelModel.ModelMatrix);
        racer.car->leftRearWheelModel.render();
        m_depthShader.loadTransformMatrix(racer.car->rightFrontWheelModel.ModelMatrix);
        racer.car->rightFrontWheelModel.render();
        m_depthShader.loadTransformMatrix(racer.car->rightRearWheelModel.ModelMatrix);
        racer.car->rightRearWheelModel.render();
        m_depthShader.loadTransformMatrix(racer.car->carBodyModel.ModelMatrix);
        racer.car->carBodyModel.render();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Config::get().resX, Config::get().resY);
    m_depthShader.unbind();
    m_depthShader.shaderSet.UpdatePrograms();
}

ShadowMapRenderer::~ShadowMapRenderer() {
    glDeleteFramebuffers(1, &m_fboDepthMap);
    m_depthShader.cleanup();
};
