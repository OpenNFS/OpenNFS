//
// Created by amrik on 01/03/19.
//

#include "RaceSession.h"

void RaceSession::simulate() {
   /* // Skip CAN animation of PS1 track loaded
    bool cameraAnimationPlayed = track->tag == NFS_3_PS1;

    bool newAssetSelected = false;
    bool entityTargeted = false;
    Entity *targetedEntity;

    while (!glfwWindowShouldClose(window)) {
        // glfwGetTime is called only once, the first time this function is called
        static double lastTime = glfwGetTime();
        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        // Update time between engine ticks
        auto deltaTime = float(currentTime - lastTime); // Keep track of time between engine ticks

        // Step the physics simulation and update physics debug view matrices
        physicsEngine.mydebugdrawer.SetMatrices(mainCamera.ViewMatrix, mainCamera.ProjectionMatrix);
        physicsEngine.stepSimulation(deltaTime);

        // Play the original camera animation
        if (!cameraAnimationPlayed) {
            cameraAnimationPlayed = mainCamera.playAnimation();
        } else if (userParams.attachCamToHermite) {
            mainCamera.useSpline(totalTime);
        } else if (userParams.attachCamToCar) {
            // Compute MVP from keyboard and mouse, centered around a target car
            mainCamera.followCar(car, userParams.windowActive);
        } else {
            // Compute the MVP matrix from keyboard and mouse input
            mainCamera.computeMatricesFromInputs(userParams.windowActive, deltaTime);
        }

        //TODO: Refactor to controller class? AND USE SDL
        if (userParams.simulate_car) {
            // TODO: The AI should go through a list of CarAgents
            //car->simulate();
        } else {
            if (userParams.windowActive && !ImGui::GetIO().MouseDown[1]) {
                car->applyAccelerationForce(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS, glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
                car->applyBrakingForce(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
                car->applySteeringRight(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
                car->applySteeringLeft(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
            }
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            CarAgent::resetToVroad(closestBlockID, track, car);
        }

        // Hot reload shaders
        UpdateShaders();
        NewFrame();


        std::vector<int> activeTrackBlockIDs;
        if (userParams.frustumCull) {
            physicsEngine.updateFrustrum(mainCamera.ViewMatrix);
            // Iterate through visible entity list, based on frustum intersection
            for (int i = 0; i < physicsEngine.m_objectsInFrustum.size(); ++i) {
                auto *visibleEntity = static_cast<Entity *>(physicsEngine.m_objectsInFrustum[i]->getUserPointer());
                if (visibleEntity->type == ROAD) {
                    activeTrackBlockIDs.emplace_back(visibleEntity->parentTrackblockID);
                }
            }
        } else {
            physicsEngine.destroyGhostObject();
            activeTrackBlockIDs = CullTrackBlocks(userParams.attachCamToHermite ? mainCamera.position : userParams.attachCamToCar
                                                                                                        ? car->car_body_model.position
                                                                                                        : mainCamera.position,
                                                  userParams.blockDrawDistance, userParams.useNbData);
        }

        // If Sun moving below Horizon, change 'Sun' to 'Moon' and flip some state so we know to drop ambient in TrackShader
        bool nightTime = UpdateGlobalLights();
        float ambientLightFactor = nightTime ? 0.2f : 0.5f;

        shadowMapRenderer.renderShadowMap(nightTime ? moon.ViewMatrix : sun.ViewMatrix, activeTrackBlockIDs, car);
        skyRenderer.renderSky(mainCamera, sun, userParams, totalTime);
        trackRenderer.renderTrack(mainCamera, nightTime ? moon : sun, activeTrackBlockIDs, userParams, ticks, shadowMapRenderer.depthTextureID, shadowMapRenderer.lightSpaceMatrix, ambientLightFactor);
        trackRenderer.renderLights(mainCamera, activeTrackBlockIDs);

        // Render the Car
        if (car->tag == NFS_3 || car->tag == NFS_4) SetCulling(true);
        // Get lights that will contribute to car body (currentBlock, a few blocks forward, and a few back (NBData would give weird results, as NBData blocks aren't generally adjacent))
        // Should use NFS3/4 Shading data too as a fake light
        std::vector<Light> carBodyContributingLights = trackRenderer.trackLightMap[closestBlockID];
        carBodyContributingLights.emplace_back(nightTime ? moon : sun);
        carRenderer.render(mainCamera, carBodyContributingLights);
        SetCulling(false);

        if (userParams.drawRaycast) {
            DrawCarRaycasts();
        }

        if (userParams.drawCAN) {
            DrawCameraAnimation();
        }

        if (userParams.drawVroad) {
            DrawVroad();
        }

        if (ImGui::GetIO().MouseReleased[0] & userParams.windowActive) {
            targetedEntity = physicsEngine.checkForPicking(mainCamera.ViewMatrix, mainCamera.ProjectionMatrix, &entityTargeted);
        }

        if (entityTargeted) {
            DrawMetadata(targetedEntity);
        }

        if (userParams.physicsDebugView) {
            physicsEngine.getDynamicsWorld()->debugDrawWorld();
        }

        if (DrawMenuBar()) {
            newAssetSelected = true;
        };

        DrawUI(mainCamera.position);
        glfwSwapBuffers(window);

        if (newAssetSelected) break;

        // For the next frame, the "last time" will be "now"
        lastTime = currentTime;
        // Keep track of total elapsed time too
        totalTime += deltaTime;
        ++ticks;
    }
    if (newAssetSelected) {
        return loadedAssets;
    } else {
        // Just set a flag temporarily to let main know that we outta here
        loadedAssets.trackTag = UNKNOWN;
        return loadedAssets;
    }*/
}
