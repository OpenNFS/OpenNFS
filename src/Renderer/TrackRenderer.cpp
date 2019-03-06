//
// Created by Amrik Sadhra on 21/08/2018.
//

#include "TrackRenderer.h"

TrackRenderer::TrackRenderer(const std::shared_ptr<ONFSTrack> &activeTrack) {
    track = activeTrack;
    trackLightMap = GetContributingLights(activeTrack);
}

// Return a vector of Lights that will contribute interestingly to final diffuse/spec (COLOUR)
std::vector<Light> TrackRenderer::GetInterestingLights(const TrackBlock &activeTrackBlock) {
    std::vector<Light> interestingLights;
    std::vector<Light> boringLights;
    int nLights = 0;

    for (auto &light_entity : activeTrackBlock.lights) {
        Light currentLight = boost::get<Light>(light_entity.glMesh);
        glm::vec4 colour = currentLight.colour;

        // Try to get coloured lights first. 3.f total for rgb components is a 255, 255, 255 whitewash
        if ((colour.y + colour.z + colour.w) < 2.5f) {
            if (++nLights < LIGHTS_PER_NB_BLOCK) {
                interestingLights.emplace_back(currentLight);
            } else {
                return interestingLights;
            }
        } else {
            boringLights.emplace_back(currentLight);
        }
    }

    // Backfill with the boring lights now
    for (auto boringLight : boringLights) {
        if (interestingLights.size() < LIGHTS_PER_NB_BLOCK) {
            interestingLights.emplace_back(boringLight);
        }
    }

    return interestingLights;
}

std::map<int, std::vector<Light>> TrackRenderer::GetContributingLights(const std::shared_ptr<ONFSTrack> &activeTrack) {
    // Must leave room for sun light to be added.
    ASSERT(LIGHTS_PER_NB_BLOCK * ((2 * NEIGHBOUR_BLOCKS_FOR_LIGHTS) + 1) <= MAX_TRACK_CONTRIB_LIGHTS - 1,
           "With current number of lights being considered from number of neighbouring blocks, more lights will be bound to the shader than can be processed! Increase MAX_TRACK_CONTRIB_LIGHTS");

    std::map<int, std::vector<Light>> trackToLightMap;

    for (int trackBlk_Idx = 0; trackBlk_Idx < activeTrack->nBlocks; ++trackBlk_Idx) {
        std::vector<Light> contribLightsForCurrentBlock;

        // Get list of blocks on either side of current trackblock by nContributingBlocks
        for (int block_Idx = trackBlk_Idx - NEIGHBOUR_BLOCKS_FOR_LIGHTS;
             block_Idx < trackBlk_Idx + NEIGHBOUR_BLOCKS_FOR_LIGHTS; ++block_Idx) {
            int activeBlock = block_Idx < 0 ? ((int) activeTrack->trackBlocks.size() + block_Idx) : (block_Idx %
                                                                                                      (int) activeTrack->trackBlocks.size());

            std::vector<Light> contribLightsForNeighbouringBlock = GetInterestingLights(
                    activeTrack->trackBlocks[activeBlock]);
            contribLightsForCurrentBlock.insert(contribLightsForCurrentBlock.begin(),
                                                contribLightsForNeighbouringBlock.begin(),
                                                contribLightsForNeighbouringBlock.end());
        }

        // Cache this result per trackblock
        trackToLightMap[trackBlk_Idx] = contribLightsForCurrentBlock;
    }

    return trackToLightMap;
}


void TrackRenderer::renderTrack(const Camera &mainCamera, const Light &sunLight,
                                std::vector<int> activeTrackBlockIDs, const ParamData &userParams,
                                GLuint depthTextureID, const glm::mat4 &lightSpaceMatrix, float ambientFactor) {
    trackShader.use();

    // This shader state doesnt change during a track renderpass
    trackShader.setClassic(userParams.useClassicGraphics);
    trackShader.loadProjectionViewMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix);
    trackShader.loadLightSpaceMatrix(lightSpaceMatrix);
    trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
    trackShader.bindTextureArray(track->textureArrayID);
    trackShader.loadShadowMapTexture(depthTextureID);
    trackShader.loadAmbientFactor(ambientFactor);

    std::vector<Light> globalLights;
    globalLights.emplace_back(sunLight);

    // Render the per-trackblock data
    for (int activeTrackBlockID : activeTrackBlockIDs) {
        TrackBlock active_track_Block = track->trackBlocks[activeTrackBlockID];
        std::vector<Light> contributingLights = trackLightMap[activeTrackBlockID];
        contributingLights.emplace_back(sunLight);
        trackShader.loadLights(contributingLights);
        for (auto &track_block_entity : active_track_Block.track) {
            trackShader.loadTransformMatrix(boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
        for (auto &track_block_entity : active_track_Block.objects) {
            trackShader.loadTransformMatrix(boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
        // Could render Lanes with a simpler shader set, straight vert MVP transform w/ one texture sample on bound lane texture
        // Probably not worth the overhead of switching GL state
        for (auto &track_block_entity : active_track_Block.lanes) {
            trackShader.loadTransformMatrix(boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
            boost::get<Track>(track_block_entity.glMesh).render();
        }
    }

    // Render the global data, animations go here.
    for (auto &global_object : track->globalObjects) {
        if (track->tag == NFS_4 || track->tag == NFS_3) {
            uint32_t globalObjIdx = 4 * track->nBlocks; //Global Objects
            NFS3_4_DATA::XOBJDATA animObject = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(
                    track->trackData)->xobj[globalObjIdx].obj[global_object.entityID];
            if (animObject.type3 == 3) {
                if (animMap[global_object.entityID] < animObject.nAnimLength) {
                    boost::get<Track>(global_object.glMesh).position =
                            glm::normalize(glm::quat(glm::vec3(glm::radians(-90.f), 0, 0))) *
                            glm::vec3((animObject.animData[animMap[global_object.entityID]].pt.x / 65536.0) / 10,
                                      (animObject.animData[animMap[global_object.entityID]].pt.y / 65536.0) / 10,
                                      (animObject.animData[animMap[global_object.entityID]].pt.z / 65536.0) / 10);
                    boost::get<Track>(global_object.glMesh).orientation =
                            glm::normalize(glm::quat(glm::vec3(glm::radians(-180.f), glm::radians(-180.f), 0))) *
                            glm::normalize(glm::quat(-animObject.animData[animMap[global_object.entityID]].od1,
                                                     animObject.animData[animMap[global_object.entityID]].od2,
                                                     animObject.animData[animMap[global_object.entityID]].od3,
                                                     animObject.animData[animMap[global_object.entityID]].od4));
                    animMap[global_object.entityID]++;
                } else {
                    animMap[global_object.entityID] = 0;
                }
            }
        } else if (track->tag == NFS_2 || track->tag == NFS_2_SE || track->tag == NFS_3_PS1) {
            std::vector<GEOM_REF_BLOCK> colStructureRefData =
                    track->tag == NFS_3_PS1 ? boost::get<std::shared_ptr<NFS2_DATA::PS1::TRACK>>(
                            track->trackData)->colStructureRefData : boost::get<std::shared_ptr<NFS2_DATA::PC::TRACK>>(
                            track->trackData)->colStructureRefData;
            // Find the structure reference that matches this structure, else use block default
            for (auto &structure : colStructureRefData) {
                // Only check fixed type structure references
                if (structure.structureRef == global_object.entityID) {
                    if (structure.recType == 3) {
                        if (animMap[global_object.entityID] < structure.animLength) {
                            boost::get<Track>(global_object.glMesh).position =
                                    glm::normalize(glm::quat(glm::vec3(glm::radians(-90.f), 0, 0))) * glm::vec3(
                                            structure.animationData[animMap[global_object.entityID]].position.x /
                                            1000000.0f,
                                            structure.animationData[animMap[global_object.entityID]].position.y /
                                            1000000.0f,
                                            structure.animationData[animMap[global_object.entityID]].position.z /
                                            1000000.0f);
                            boost::get<Track>(global_object.glMesh).orientation =
                                    glm::normalize(glm::quat(glm::vec3(glm::radians(-180.f), 0, 0))) * glm::normalize(
                                            glm::quat(
                                                    -structure.animationData[animMap[global_object.entityID]].unknown[0],
                                                    structure.animationData[animMap[global_object.entityID]].unknown[1],
                                                    structure.animationData[animMap[global_object.entityID]].unknown[2],
                                                    structure.animationData[animMap[global_object.entityID]].unknown[3]));
                            animMap[global_object.entityID]++;
                        } else {
                            animMap[global_object.entityID] = 0;
                        }
                    }
                }
            }
        }
        boost::get<Track>(global_object.glMesh).update();
        trackShader.loadTransformMatrix(boost::get<Track>(global_object.glMesh).ModelMatrix);
        trackShader.loadLights(globalLights);
        boost::get<Track>(global_object.glMesh).render();
    }
    trackShader.unbind();
}

void TrackRenderer::renderLights(const Camera &mainCamera, std::vector<int> activeTrackBlockIDs) {
    billboardShader.use();
    for (auto &track_block_id : activeTrackBlockIDs) {
        // Render the lights far to near
        for (auto &light_entity : std::vector<Entity>(track->trackBlocks[track_block_id].lights.rbegin(),
                                                      track->trackBlocks[track_block_id].lights.rend())) {
            billboardShader.loadMatrices(mainCamera.ProjectionMatrix, mainCamera.ViewMatrix,
                                         boost::get<Light>(light_entity.glMesh).ModelMatrix);
            billboardShader.loadLight(boost::get<Light>(light_entity.glMesh));
            boost::get<Light>(light_entity.glMesh).render();
        }
    }
    billboardShader.unbind();
}

TrackRenderer::~TrackRenderer() {
    // Cleanup VBOs and shaders
    trackShader.cleanup();
}



