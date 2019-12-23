#include "TrackRenderer.h"

void TrackRenderer::Render(shared_ptr<Car> &car, const Camera &mainCamera, const GlobalLight &light,
                                const shared_ptr<ONFSTrack> &track, const ParamData &userParams,
                                GLuint depthTextureID, float ambientFactor) {
    trackShader.use();
    // This shader state doesnt change during a track renderpass
    trackShader.setClassic(userParams.useClassicGraphics);
    trackShader.loadProjectionViewMatrices(mainCamera.projectionMatrix, mainCamera.viewMatrix);
    trackShader.loadLightSpaceMatrix(light.lightSpaceMatrix);
    trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
    trackShader.bindTextureArray(track->textureArrayID);
    trackShader.loadShadowMapTexture(depthTextureID);
    trackShader.loadAmbientFactor(ambientFactor);
    trackShader.loadSpotlight(car->leftHeadlight);

    // Render the per-trackblock data
    for (auto &trackBlock : track->trackBlocks) {
        // trackShader.loadLights(trackBlock.lights);
        for (auto &track_block_entity : trackBlock.track) {
            trackShader.loadTransformMatrix(boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
        for (auto &track_block_entity : trackBlock.objects) {
            trackShader.loadTransformMatrix(boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
        // Could render Lanes with a simpler shader set, straight vert MVP transform w/ one texture sample on bound lane texture
        // Probably not worth the overhead of switching GL state
        for (auto &track_block_entity : trackBlock.lanes) {
            trackShader.loadTransformMatrix(boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
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



