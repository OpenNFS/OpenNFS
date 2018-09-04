//
// Created by Amrik Sadhra on 21/08/2018.
//

#include "TrackRenderer.h"

TrackRenderer::TrackRenderer(const shared_ptr<ONFSTrack> &activeTrack) {
    track = activeTrack;
}

void TrackRenderer::renderTrack(const Camera &mainCamera, const Light &cameraLight, std::vector<int> activeTrackBlockIDs, const ParamData &userParams) {
    std::vector<Light> camlights;
    camlights.push_back(cameraLight);

    // TODO: Urgently refactor these into Renderer classes
    // Render the per-trackblock data
    for (int activeBlk_Idx = 0; activeBlk_Idx < activeTrackBlockIDs.size(); ++activeBlk_Idx) {
        TrackBlock active_track_Block = track->track_blocks[activeTrackBlockIDs[activeBlk_Idx]];
        std::vector<Entity> contributingLightEntities = active_track_Block.lights;
        std::vector<Light> contributingLights;
        // TODO: Merge lighting contributions across track block, must use a smarter Track structure, also must be a better way of building this from the Entities. This will be too slow.
        for (auto &light_entity : contributingLightEntities) {
            contributingLights.emplace_back(boost::get<Light>(light_entity.glMesh));
        }
        trackShader.use();
        for (auto &track_block_entity : active_track_Block.objects) {
            boost::get<Track>(track_block_entity.glMesh).update();
            trackShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
            if (contributingLightEntities.size() > 0) {
                trackShader.loadLights(contributingLights);
            } else {
                trackShader.loadLights(camlights);
            }
            trackShader.bindTrackTextures(boost::get<Track>(track_block_entity.glMesh), track->texture_gl_mappings);
            trackShader.setClassic(userParams.use_classic_graphics);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
        for (auto &track_block_entity : active_track_Block.track) {
            boost::get<Track>(track_block_entity.glMesh).update();
            trackShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
            if (contributingLightEntities.size() > 0) {
                trackShader.loadLights(contributingLights);
            } else {
                trackShader.loadLights(camlights);
            }
            trackShader.bindTrackTextures(boost::get<Track>(track_block_entity.glMesh), track->texture_gl_mappings);
            trackShader.setClassic(userParams.use_classic_graphics);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
        // TODO: Render Lanes with a simpler shader set
        for (auto &track_block_entity : active_track_Block.lanes) {
            boost::get<Track>(track_block_entity.glMesh).update();
            trackShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
            if (contributingLightEntities.size() > 0) {
                trackShader.loadLights(contributingLights);
            } else {
                trackShader.loadLights(camlights);
            }
            trackShader.bindTrackTextures(boost::get<Track>(track_block_entity.glMesh), track->texture_gl_mappings);
            trackShader.setClassic(userParams.use_classic_graphics);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
        trackShader.unbind();
    }

    // Render the global data, animations go here.
    trackShader.use();
    for (auto &global_object : track->global_objects) {
        if (track->tag == NFS_3) {
            COLFILE col = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col;
            if (col.object[global_object.entityID].type == 3) {
                if (animMap[global_object.entityID] < col.object[global_object.entityID].animLength) {
                    boost::get<Track>(global_object.glMesh).position = glm::normalize(glm::quat(glm::vec3(glm::radians(-90.f), 0, 0))) * glm::vec3((col.object[global_object.entityID].animData[animMap[global_object.entityID]].pt.x / 65536.0) / 10, (col.object[global_object.entityID].animData[animMap[global_object.entityID]].pt.y / 65536.0) / 10, (col.object[global_object.entityID].animData[animMap[global_object.entityID]].pt.z / 65536.0) / 10);
                    boost::get<Track>(global_object.glMesh).orientation = glm::normalize(glm::quat(glm::vec3(glm::radians(-180.f), glm::radians(-180.f), 0))) * glm::normalize(glm::quat(-col.object[global_object.entityID].animData[animMap[global_object.entityID]].od1, col.object[global_object.entityID].animData[animMap[global_object.entityID]].od2, col.object[global_object.entityID].animData[animMap[global_object.entityID]].od3, col.object[global_object.entityID].animData[animMap[global_object.entityID]].od4));
                    animMap[global_object.entityID]++;
                } else {
                    animMap[global_object.entityID] = 0;
                }
            }
        } else if (track->tag == NFS_2 || track->tag == NFS_2_SE) {
            // Find the structure reference that matches this structure, else use block default
            for (auto &structure : boost::get<shared_ptr<NFS2_DATA::PC::TRACK>>(track->trackData)->colStructureRefData) {
                // Only check fixed type structure references
                if (structure.structureRef == global_object.entityID) {
                    if (structure.recType == 3) {
                        if (animMap[global_object.entityID] < structure.animLength) {
                            boost::get<Track>(global_object.glMesh).position = glm::normalize(glm::quat(glm::vec3(glm::radians(-90.f), 0, 0))) * glm::vec3(structure.animationData[animMap[global_object.entityID]].position.x/1000000.0f, structure.animationData[animMap[global_object.entityID]].position.y/1000000.0f, structure.animationData[animMap[global_object.entityID]].position.z/1000000.0f);
                            boost::get<Track>(global_object.glMesh).orientation = glm::normalize(glm::quat(glm::vec3(glm::radians(-180.f), 0, 0))) * glm::normalize(glm::quat(-structure.animationData[animMap[global_object.entityID]].unknown[0],structure.animationData[animMap[global_object.entityID]].unknown[1], structure.animationData[animMap[global_object.entityID]].unknown[2], structure.animationData[animMap[global_object.entityID]].unknown[3]));
                            animMap[global_object.entityID]++;
                        } else {
                            animMap[global_object.entityID] = 0;
                        }
                    }
                }
            }
        } else if (track->tag == NFS_3_PS1) {
            // Find the structure reference that matches this structure, else use block default
            for (auto &structure : boost::get<shared_ptr<NFS2_DATA::PS1::TRACK>>(track->trackData)->colStructureRefData) {
                // Only check fixed type structure references
                if (structure.structureRef == global_object.entityID) {
                    if (structure.recType == 3) {
                        if (animMap[global_object.entityID] < structure.animLength) {
                            boost::get<Track>(global_object.glMesh).position = glm::normalize(glm::quat(glm::vec3(glm::radians(-90.f), 0, 0))) * glm::vec3(structure.animationData[animMap[global_object.entityID]].position.x/1000000.0f, structure.animationData[animMap[global_object.entityID]].position.y/1000000.0f, structure.animationData[animMap[global_object.entityID]].position.z/1000000.0f);
                            boost::get<Track>(global_object.glMesh).orientation = glm::normalize(glm::quat(glm::vec3(glm::radians(-180.f), 0, 0))) * glm::normalize(glm::quat(-structure.animationData[animMap[global_object.entityID]].unknown[0],structure.animationData[animMap[global_object.entityID]].unknown[1], structure.animationData[animMap[global_object.entityID]].unknown[2], structure.animationData[animMap[global_object.entityID]].unknown[3]));
                            animMap[global_object.entityID]++;
                        } else {
                            animMap[global_object.entityID] = 0;
                        }
                    }
                }
            }
        }
        boost::get<Track>(global_object.glMesh).update();
        trackShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, boost::get<Track>(global_object.glMesh).ModelMatrix);
        trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
        trackShader.loadLights(camlights);
        trackShader.bindTrackTextures(boost::get<Track>(global_object.glMesh), track->texture_gl_mappings);
        trackShader.setClassic(userParams.use_classic_graphics);
        boost::get<Track>(global_object.glMesh).render();
    }
    trackShader.unbind();
}

TrackRenderer::~TrackRenderer() {
    // Cleanup VBOs and shaders
    trackShader.cleanup();
}

void TrackRenderer::renderLights(const Camera &mainCamera, std::vector<int> activeTrackBlockIDs) {
    for (auto &track_block_id : activeTrackBlockIDs) {
        billboardShader.use();
        // Render the lights far to near
        for (auto &light_entity : std::vector<Entity>(track->track_blocks[track_block_id].lights.rbegin(), track->track_blocks[track_block_id].lights.rend())) {
            boost::get<Light>(light_entity.glMesh).update();
            billboardShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix, boost::get<Light>(light_entity.glMesh).ModelMatrix);
            billboardShader.loadLight(boost::get<Light>(light_entity.glMesh));
            boost::get<Light>(light_entity.glMesh).render();
        }
        billboardShader.unbind();
    }
}

