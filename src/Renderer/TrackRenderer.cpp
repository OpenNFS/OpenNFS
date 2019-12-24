#include "TrackRenderer.h"

void TrackRenderer::Render(shared_ptr<Car> &car, const Camera &mainCamera, const GlobalLight &light, GLuint trackTextureArrayID, const std::vector<std::shared_ptr<Entity>> &visibleEntities, const ParamData &userParams,
                                GLuint depthTextureID, float ambientFactor) {
    trackShader.use();
    // This shader state doesnt change during a track renderpass
    trackShader.setClassic(userParams.useClassicGraphics);
    trackShader.loadProjectionViewMatrices(mainCamera.projectionMatrix, mainCamera.viewMatrix);
    trackShader.loadLightSpaceMatrix(light.lightSpaceMatrix);
    trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
    trackShader.bindTextureArray(trackTextureArrayID);
    trackShader.loadShadowMapTexture(depthTextureID);
    trackShader.loadAmbientFactor(ambientFactor);
    trackShader.loadSpotlight(car->leftHeadlight);

    // Render the per-trackblock data
    for (auto &entity : visibleEntities) {
        /*std::vector<Light> lights;
        for(auto &lightEntity : trackBlock.lights)
        {
            // TODO: This is super silly.
            lights.emplace_back(boost::get<Light>(lightEntity.glMesh));
        }
        trackShader.loadLights(lights);*/
        trackShader.loadTransformMatrix(boost::get<Track>(entity->glMesh).ModelMatrix);
        boost::get<Track>(entity->glMesh).render();
    }

    trackShader.unbind();
    trackShader.shaderSet.UpdatePrograms();
}

void TrackRenderer::RenderLights(const Camera &mainCamera, const shared_ptr<ONFSTrack> &track) {
    billboardShader.use();
    for (auto &trackBlock : track->trackBlocks) {
        // Render the lights far to near
        for (auto &lightEntity : trackBlock.lights) {
            billboardShader.loadMatrices(mainCamera.projectionMatrix, mainCamera.viewMatrix, boost::get<Light>(lightEntity.glMesh).ModelMatrix);
            billboardShader.loadLight(boost::get<Light>(lightEntity.glMesh));
            boost::get<Light>(lightEntity.glMesh).render();
        }
    }
    billboardShader.unbind();
    billboardShader.shaderSet.UpdatePrograms();
}

TrackRenderer::~TrackRenderer() {
    // Cleanup VBOs and shaders
    trackShader.cleanup();
    billboardShader.cleanup();
}



