#include "TrackRenderer.h"

void TrackRenderer::Render(const std::vector<CarAgent> &racers, const std::shared_ptr<Camera> &camera, const GlobalLight &light, GLuint trackTextureArrayID, const std::vector<std::shared_ptr<Entity>> &visibleEntities, const ParamData &userParams,
                                GLuint depthTextureID, float ambientFactor) {
    m_trackShader.use();
    // This shader state doesnt change during a track renderpass
    m_trackShader.setClassic(userParams.useClassicGraphics);
    m_trackShader.loadProjectionViewMatrices(camera->projectionMatrix, camera->viewMatrix);
    m_trackShader.loadLightSpaceMatrix(light.lightSpaceMatrix);
    m_trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
    m_trackShader.bindTextureArray(trackTextureArrayID);
    m_trackShader.loadShadowMapTexture(depthTextureID);
    m_trackShader.loadAmbientFactor(ambientFactor);
    //m_trackShader.loadSpotlight(car->leftHeadlight);

    // Render the per-trackblock data
    for (auto &entity : visibleEntities) {
        /*std::vector<Light> lights;
        for(auto &lightEntity : trackBlock.lights)
        {
            // TODO: This is super silly.
            lights.emplace_back(boost::get<Light>(lightEntity.glMesh));
        }
        trackShader.loadLights(lights);*/
        m_trackShader.loadTransformMatrix(boost::get<Track>(entity->glMesh).ModelMatrix);
        boost::get<Track>(entity->glMesh).render();
    }

    m_trackShader.unbind();
    m_trackShader.shaderSet.UpdatePrograms();
}

void TrackRenderer::RenderLights(const std::shared_ptr<Camera> &camera, const shared_ptr<ONFSTrack> &track) {
    m_billboardShader.use();
    for (auto &trackBlock : track->trackBlocks) {
        // Render the lights far to near
        for (auto &lightEntity : trackBlock.lights) {
            m_billboardShader.loadMatrices(camera->projectionMatrix, camera->viewMatrix, boost::get<Light>(lightEntity.glMesh).ModelMatrix);
            m_billboardShader.loadLight(boost::get<Light>(lightEntity.glMesh));
            boost::get<Light>(lightEntity.glMesh).render();
        }
    }
    m_billboardShader.unbind();
    m_billboardShader.shaderSet.UpdatePrograms();
}

TrackRenderer::~TrackRenderer() {
    // Cleanup VBOs and shaders
    m_trackShader.cleanup();
    m_billboardShader.cleanup();
}



