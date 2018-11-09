//
// Created by amrik on 08/11/18.
//

#include "ShadowMapRenderer.h"

ShadowMapRenderer::ShadowMapRenderer(const shared_ptr<ONFSTrack> &activeTrack): track(activeTrack) {
    // -----------------------
    // Configure depth map FBO
    // -----------------------
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    glGenTextures(1, &depthTextureID);
    glBindTexture(GL_TEXTURE_2D, depthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);*/
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Always check that our framebuffer is ok
    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Depth FBO is nae good.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapRenderer::renderShadowMap(const glm::mat4 &lightViewMatrix,  std::vector<int> activeTrackBlockIDs, const std::shared_ptr<Car> &car){
    /* ------- SHADOW MAPPING ------- */
    glCullFace(GL_FRONT);
    depthShader.use();
    float near_plane = 160.0f, far_plane = 280.f;
    glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
    lightSpaceMatrix = lightProjection * lightViewMatrix;

    depthShader.loadLightSpaceMatrix(lightSpaceMatrix);
    depthShader.bindTextureArray(track->textureArrayID);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    /* Render the track using this simple shader to get depth texture to test against during draw */
    for (int activeBlk_Idx = 0; activeBlk_Idx < activeTrackBlockIDs.size(); ++activeBlk_Idx) {
        TrackBlock active_track_Block = track->track_blocks[activeTrackBlockIDs[activeBlk_Idx]];
        std::vector<Light> contributingLights;
        for (auto &light_entity : active_track_Block.lights) {
            contributingLights.emplace_back(boost::get<Light>(light_entity.glMesh));
        }
        for (auto &track_block_entity : active_track_Block.track) {
            boost::get<Track>(track_block_entity.glMesh).update();
            depthShader.loadTransformMatrix(boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
        for (auto &track_block_entity : active_track_Block.objects) {
            boost::get<Track>(track_block_entity.glMesh).update();
            depthShader.loadTransformMatrix(boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
    }
    /* And the Car */
    depthShader.bindTextureArray(car->textureArrayID);
    for (auto &misc_model : car->misc_models) {
        depthShader.loadTransformMatrix(misc_model.ModelMatrix);
        misc_model.render();
    }
    depthShader.loadTransformMatrix(car->left_front_wheel_model.ModelMatrix);
    car->left_front_wheel_model.render();
    depthShader.loadTransformMatrix(car->left_rear_wheel_model.ModelMatrix);
    car->left_rear_wheel_model.render();
    depthShader.loadTransformMatrix(car->right_front_wheel_model.ModelMatrix);
    car->right_front_wheel_model.render();
    depthShader.loadTransformMatrix(car->right_rear_wheel_model.ModelMatrix);
    car->right_rear_wheel_model.render();
    depthShader.loadTransformMatrix(car->car_body_model.ModelMatrix);
    car->car_body_model.render();

    glCullFace(GL_BACK); // Reset original culling face
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, 1920, 1080);
}

ShadowMapRenderer::~ShadowMapRenderer() {
    glDeleteFramebuffers(1, &depthMapFBO);
    depthShader.cleanup();
};
