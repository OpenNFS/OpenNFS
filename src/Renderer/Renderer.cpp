//
// Created by Amrik Sadhra on 19/07/2018.
//


#include "Renderer.h"


Renderer::Renderer(GLFWwindow *gl_window, std::shared_ptr<Logger> onfs_logger, const std::vector<NeedForSpeed> &installedNFS, const shared_ptr<ONFSTrack> &current_track, shared_ptr<Car> current_car) : carRenderer(current_car), trackRenderer(current_track), skyRenderer(current_track) {
    window = gl_window;
    logger = onfs_logger;

    installedNFSGames = installedNFS;
    track = current_track;
    car = current_car;

    loadedAssets.carTag = car->tag;
    loadedAssets.car = car->name;
    loadedAssets.trackTag = track->tag;
    loadedAssets.track = track->name;

    glm::vec3 initialCameraPosition;
    if (track->tag == NFS_3_PS1) {
        initialCameraPosition = glm::vec3(track->track_blocks[0].center.x, track->track_blocks[0].center.y, track->track_blocks[0].center.z);
    } else {
        // VROAD POINT 0 wont be available for NFS2
        /*INTPT refPt =  boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[0].refPt;
        glm::vec3 vroadPoint = glm::normalize(glm::quat(glm::vec3(-SIMD_PI/2,0,0))) * glm::vec3((refPt.x/ 65536.0f) / 10.f, ((refPt.y/ 65536.0f) / 10.f), (refPt.z/ 65536.0f) / 10.f);
        vroadPoint += 0.2;*/
        glm::vec3 vroadPoint = glm::vec3(track->track_blocks[0].center.x, track->track_blocks[0].center.y, track->track_blocks[0].center.z);
        initialCameraPosition = vroadPoint;
    }

    mainCamera = Camera(initialCameraPosition, 55.0f, 4.86f, -0.21f, window);
    mainCamera.generateSpline(track->track_blocks);
    cameraLight = Light(mainCamera.position, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), 1, 0, 0, 0, 0.f);

    // Generate the collision meshes
    physicsEngine.registerTrack(track);
    physicsEngine.registerVehicle(car);

    /*------- ImGui -------*/
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    // Create Depth buffer and attach
    InitialiseDepthTexture();

    LOG(DEBUG) << "Renderer Initialised";
}

void Renderer::InitialiseDepthTexture(){
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

AssetData Renderer::Render() {
    ParamData userParams;

    // Skip CAN animation of PS1 track loaded
    bool camera_animation_played = track->tag == NFS_3_PS1;

    if (track->tag == NFS_2_SE || track->tag == NFS_2 || track->tag == NFS_3_PS1) {
        userParams.use_nb_data = false;
    }

    if (track->tag != NFS_3_PS1) {
        mainCamera.setCameraAnimation(track->camera_animations);
    }

    Light sun = Light(glm::vec3(0, 200, 0), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);
    sun.attenuation.x = 0.1;
    Light moon = Light(glm::vec3(0, -200, 0), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);
    moon.attenuation.x = 0.1;

    // Detect position change to trigger Cull code
    glm::vec3 oldWorldPosition(0, 0, 0);

    car->resetCar(glm::vec3(track->track_blocks[0].center.x, track->track_blocks[0].center.y, track->track_blocks[0].center.z));

    bool entity_targeted = false;
    Entity *targetedEntity;

    bool newAssetSelected = false;

    while (!glfwWindowShouldClose(window)) {
        // glfwGetTime is called only once, the first time this function is called
        static double lastTime = glfwGetTime();
        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        // Update time between engine ticks
        float deltaTime = float(currentTime - lastTime); // Keep track of time between engine ticks

        NewFrame(&userParams);

        // Play the original camera animation
        if (!camera_animation_played) {
            camera_animation_played = mainCamera.playAnimation();
        } else if (userParams.attach_cam_to_hermite) {
            mainCamera.useSpline(totalTime);
        } else if (userParams.attach_cam_to_car) {
            // Compute MVP from keyboard and mouse, centered around a target car
            mainCamera.followCar(car, userParams.window_active, ImGui::GetIO());
        } else {
            // Compute the MVP matrix from keyboard and mouse input
            mainCamera.computeMatricesFromInputs(userParams.window_active, ImGui::GetIO(), deltaTime);
        }

        physicsEngine.mydebugdrawer.SetMatrices(mainCamera.ViewMatrix, mainCamera.ProjectionMatrix);

        //TODO: Refactor to controller class? AND USE SDL
        if (userParams.simulate_car) {
            car->simulate();
        } else {
            if (userParams.window_active && !ImGui::GetIO().MouseDown[1]) {
                car->applyAccelerationForce(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS, glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
                car->applyBrakingForce(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
                car->applySteeringRight(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
                car->applySteeringLeft(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
            }
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            // Go and find the Vroad Data to reset to
            if (track->tag == NFS_3 || track->tag == NFS_4) {
                car->resetCar(glm::vec3(track->track_blocks[closestBlockID].center.x, (track->track_blocks[closestBlockID].center.y) + 0.2, track->track_blocks[closestBlockID].center.z));
            } else if (track->tag == NFS_2_SE || track->tag == NFS_2 || track->tag == NFS_3_PS1) {
                car->resetCar(glm::vec3(track->track_blocks[closestBlockID].center.x, (track->track_blocks[closestBlockID].center.y + 0.2), track->track_blocks[closestBlockID].center.z));
            }
        }

        // Step the physics simulation
        physicsEngine.stepSimulation(deltaTime);

        if (userParams.draw_raycast) {
            physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(car->car_body_model.position), Utils::glmToBullet(car->forwardCastPosition), btVector3(2.0f * (1.0f - car->forwardDistance), 2.0f * (car->forwardDistance), 0));
            physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(car->car_body_model.position), Utils::glmToBullet(car->upCastPosition), btVector3(2.0f * (1.0f - car->upDistance), 2.0f * (car->upDistance), 0));
            physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(car->car_body_model.position), Utils::glmToBullet(car->rightCastPosition), btVector3(2.0f * (1.0f - car->rightDistance), 2.0f * (car->rightDistance), 0));
            physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(car->car_body_model.position), Utils::glmToBullet(car->leftCastPosition), btVector3(2.0f * (1.0f - car->leftDistance), 2.0f * (car->leftDistance), 0));
        }

        if (userParams.draw_can) {
            if (track->tag != NFS_3_PS1) {
                for (uint8_t can_Idx = 0; can_Idx < track->camera_animations.size(); ++can_Idx) {
                    if (can_Idx < track->camera_animations.size() - 1) {
                        // Draw CAN positions
                        SHARED::CANPT refPt = track->camera_animations[can_Idx];
                        SHARED::CANPT refPtNext = track->camera_animations[can_Idx + 1];
                        glm::vec3 vroadPoint = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) * glm::vec3((refPt.x / 65536.0f) / 10.f, ((refPt.y / 65536.0f) / 10.f), (refPt.z / 65536.0f) / 10.f);
                        glm::vec3 vroadPointNext = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) * glm::vec3((refPtNext.x / 65536.0f) / 10.f, ((refPtNext.y / 65536.0f) / 10.f), (refPtNext.z / 65536.0f) / 10.f);
                        vroadPoint.y += 0.2f;
                        vroadPointNext.y += 0.2f;
                        physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(vroadPoint + mainCamera.initialPosition), Utils::glmToBullet(vroadPointNext + mainCamera.initialPosition), btVector3(0, 1, 1));

                        // Draw Rotations
                        glm::quat RotationMatrix = glm::normalize(glm::quat(glm::vec3(glm::radians(0.f), glm::radians(-90.f), 0))) * glm::normalize(glm::quat(refPt.od1 / 65536.0f, refPt.od2 / 65536.0f, refPt.od3 / 65536.0f, refPt.od4 / 65536.0f));
                        glm::vec3 direction = glm::normalize(vroadPoint * glm::inverse(RotationMatrix));
                        physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(vroadPoint + mainCamera.initialPosition), Utils::glmToBullet(vroadPoint + mainCamera.initialPosition + direction), btVector3(0, 0.5, 0.5));
                    }
                }
            }
        }

        // TODO: Move to TrackRenderer
        if (userParams.draw_vroad) {
            if (track->tag == NFS_3 || track->tag == NFS_4) {
                float vRoadDisplayHeight = 0.2f;
                uint32_t nVroad = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;
                for (uint32_t vroad_Idx = 0; vroad_Idx < nVroad; ++vroad_Idx) {
                    // Render COL Vroad? Should I use TRK VROAD to work across HS too?
                    if (vroad_Idx < nVroad - 1) {
                        INTPT refPt = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx].refPt;
                        INTPT refPtNext = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx + 1].refPt;
                        glm::vec3 vroadPoint = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) * glm::vec3((refPt.x / 65536.0f) / 10.f, ((refPt.y / 65536.0f) / 10.f), (refPt.z / 65536.0f) / 10.f);
                        glm::vec3 vroadPointNext = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) * glm::vec3((refPtNext.x / 65536.0f) / 10.f, ((refPtNext.y / 65536.0f) / 10.f), (refPtNext.z / 65536.0f) / 10.f);
                        vroadPoint.y += vRoadDisplayHeight;
                        vroadPointNext.y += vRoadDisplayHeight;
                        physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext), btVector3(1, 0, 1));
                    }
                }
            }
        }

        std::vector<int> activeTrackBlockIDs;
        if (userParams.frustum_cull) {
            physicsEngine.updateFrustrum(mainCamera.ViewMatrix);
            // Iterate through visible entity list, based on frustum intersection
            for (int i = 0; i < physicsEngine.m_objectsInFrustum.size(); ++i) {
                Entity *visibleEntity = static_cast<Entity *>(physicsEngine.m_objectsInFrustum[i]->getUserPointer());
                if (visibleEntity->type == ROAD) {
                    activeTrackBlockIDs.emplace_back(visibleEntity->parentTrackblockID);
                }
            }
        } else {
            physicsEngine.destroyGhostObject();
            activeTrackBlockIDs = CullTrackBlocks(oldWorldPosition, mainCamera.position, userParams.blockDrawDistance, userParams.use_nb_data);
        }

        // Move the Sun, and update the position it's looking (for test)
        sun.position = sun.position * glm::normalize(glm::quat(glm::vec3(userParams.timeScaleFactor * 0.001f, 0, 0)));
        moon.position = moon.position * glm::normalize(glm::quat(glm::vec3(userParams.timeScaleFactor * 0.001f, 0, 0)));

        // If Sun moving below Horizon, change 'Sun' to 'Moon' and flip some state so we know to drop ambient in TrackShader
        bool nightTime = (sun.position.y <= 0);
        float ambientLightFactor = nightTime ? 0.05f : 0.45f;

        sun.lookAt = track->track_blocks[closestBlockID].center;
        sun.update();
        moon.lookAt = track->track_blocks[closestBlockID].center;
        moon.update();

        /* ------- SHADOW MAPPING ------- */
        glCullFace(GL_FRONT);
        depthShader.use();
        float near_plane = 150.0f, far_plane = 300.f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = nightTime ? moon.ViewMatrix : sun.ViewMatrix;
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
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

        skyRenderer.renderSky(mainCamera, sun, userParams, totalTime);

        trackRenderer.renderTrack(mainCamera, cameraLight, activeTrackBlockIDs, userParams, ticks, depthTextureID, lightSpaceMatrix, ambientLightFactor);

        trackRenderer.renderLights(mainCamera, activeTrackBlockIDs);

        // Render the Car
        if (car->tag == NFS_3 || car->tag == NFS_4) SetCulling(true);
        int nBlocksToContributeToCar = 3;
        // Get lights that will contribute to car body (currentBlock, a few blocks forward, and a few back (NBData would give weird results, as NBData blocks aren't generally adjacent))
        // Should use NFS3/4 Shading data too as a fake light
        std::vector<Light> carBodyContributingLights;
        carBodyContributingLights.emplace_back(cameraLight);
        carBodyContributingLights.emplace_back(sun);
        activeTrackBlockIDs = CullTrackBlocks(oldWorldPosition, mainCamera.position, nBlocksToContributeToCar, false);
        for (auto activeBlk_Idx : activeTrackBlockIDs) {
            TrackBlock active_track_Block = track->track_blocks[activeBlk_Idx];
            for (auto &light_entity : active_track_Block.lights) {
                carBodyContributingLights.emplace_back(boost::get<Light>(light_entity.glMesh));
            }
        }
        carRenderer.render(mainCamera, carBodyContributingLights);
        SetCulling(false);

        if (ImGui::GetIO().MouseReleased[0] & userParams.window_active) {
            targetedEntity = CheckForPicking(mainCamera.ViewMatrix, mainCamera.ProjectionMatrix, &entity_targeted);
        }

        if (entity_targeted) {
            DrawMetadata(targetedEntity);
        }

        if (userParams.physics_debug_view) {
            physicsEngine.getDynamicsWorld()->debugDrawWorld();
        }

        if (DrawMenuBar()) {
            newAssetSelected = true;
        };

        ImGui::Begin("Shadow Map");
        ImGui::Image((ImTextureID) depthTextureID, ImVec2(SHADOW_WIDTH, SHADOW_HEIGHT), ImVec2(0,0), ImVec2(1,-1));
        ImGui::End();

        DrawUI(&userParams, mainCamera.position);
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
    }
}

Entity *Renderer::CheckForPicking(glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, bool *entity_targeted) {
    glm::vec3 out_origin;
    glm::vec3 out_direction;
    ScreenPosToWorldRay(1920 / 2, 1080 / 2, 1920, 1080, ViewMatrix, ProjectionMatrix, out_origin, out_direction);
    glm::vec3 out_end = out_origin + out_direction * 1000.0f;
    btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(out_origin.x, out_origin.y, out_origin.z), btVector3(out_end.x, out_end.y, out_end.z));
    physicsEngine.getDynamicsWorld()->rayTest(btVector3(out_origin.x, out_origin.y, out_origin.z), btVector3(out_end.x, out_end.y, out_end.z), RayCallback);
    if (RayCallback.hasHit()) {
        *entity_targeted = true;
        return static_cast<Entity *>(RayCallback.m_collisionObject->getUserPointer());
    } else {
        *entity_targeted = false;
        return nullptr;
    }
}

void Renderer::DrawNFS34Metadata(Entity *targetEntity) {
    switch (targetEntity->type) {
        case EntityType::OBJ_POLY:
            break;
        case EntityType::GLOBAL:
            break;
        case EntityType::LANE:
            break;
        case EntityType::LIGHT: {
            Light *targetLight = &boost::get<Light>(targetEntity->glMesh);
            ImVec4 lightColour(targetLight->colour.x, targetLight->colour.y, targetLight->colour.z, targetLight->colour.w);
            ImVec4 lightAttenuation(targetLight->attenuation.x, targetLight->attenuation.y, targetLight->attenuation.z, 0.0f);
            // Colour, type, attenuation, position and NFS unknowns
            ImGui::ColorEdit4("Light Colour", (float *) &lightColour); // Edit 3 floats representing a color
            targetLight->colour = glm::vec4(lightColour.x, lightColour.y, lightColour.z, lightColour.w);
            ImGui::SliderFloat3("Attenuation (A, B, C)", (float *) &lightAttenuation, 0, 10.0f);
            targetLight->attenuation = glm::vec3(lightAttenuation.x, lightAttenuation.y, lightAttenuation.z);
            ImGui::Text("x: %f y: %f z: %f ", targetLight->position.x, targetLight->position.y, targetLight->position.z);
            ImGui::Separator();
            ImGui::Text("NFS Data");
            ImGui::Text("Type: %ld", targetLight->type);
            ImGui::Text("Unknowns: ");
            ImGui::Text("[1]: %d", targetLight->unknown1);
            ImGui::Text("[2]: %d", targetLight->unknown2);
            ImGui::Text("[3]: %d", targetLight->unknown3);
            ImGui::Text("[4]: %f", targetLight->unknown4);
        }
            break;
        case EntityType::ROAD:
            break;
        case EntityType::XOBJ:
            break;
        case EntityType::SOUND:
            break;
        case EntityType::CAR:
            // TODO: Allow adjustment of shader parameters here as well, and car colour
            Car *targetCar = boost::get<Car *>(targetEntity->glMesh);
            ImGui::Text("%s", targetCar->name.c_str());
            ImGui::Text("Ray Distances U: %f F: %f R: %f L: %f", targetCar->upDistance, targetCar->forwardDistance, targetCar->rightDistance, targetCar->leftDistance);
            // Physics Parameters
            ImGui::SliderFloat("Engine Force", &targetCar->gEngineForce, 0, 10000.0f);
            ImGui::SliderFloat("Breaking Force", &targetCar->gBreakingForce, 0, 1000.0f);
            ImGui::SliderFloat("Max Engine Force", &targetCar->maxEngineForce, 0, 10000.0f);
            ImGui::SliderFloat("Max Breaking Force", &targetCar->maxBreakingForce, 0, 1000.0f);
            ImGui::SliderFloat("Susp Rest.", &targetCar->suspensionRestLength, 0, 0.1f); // btScalar(0.030);
            // TODO: When this is modified, the connection points of the wheels need changing
            ImGui::SliderFloat("Susp Stiff.", &targetCar->suspensionStiffness, 0, 1000.f);
            ImGui::SliderFloat("Susp Damp.", &targetCar->suspensionDamping, 0, 1000.f);
            ImGui::SliderFloat("Susp Compr.", &targetCar->suspensionCompression, 0, 1000.f);
            ImGui::SliderFloat("Friction.", &targetCar->wheelFriction, 0, 1.f);
            ImGui::SliderFloat("Roll Infl.", &targetCar->rollInfluence, 0, 0.5);
            ImGui::SliderFloat("Steer Incr.", &targetCar->steeringIncrement, 0.f, 0.1f);
            ImGui::SliderFloat("Steer Clamp", &targetCar->steeringClamp, 0.f, 0.5f);
            // Graphics Parameters
            /*ImGui::ColorEdit3("Car Colour", (float *) &preferences->car_color);
            ImGui::SliderFloat("Car Specular Damper", &preferences->carSpecDamper, 0, 100);
            ImGui::SliderFloat("Car Specular Reflectivity", &preferences->carSpecReflectivity, 0, 10);*/
            ImGui::Text("Roll (deg) x: %f y: %f z: %f",
                        glm::eulerAngles(targetCar->car_body_model.orientation).x * 180 / SIMD_PI,
                        glm::eulerAngles(targetCar->car_body_model.orientation).y * 180 / SIMD_PI,
                        glm::eulerAngles(targetCar->car_body_model.orientation).z * 180 / SIMD_PI);

            // TODO: Only do this on a change
            for (int i = 0; i < car->getRaycast()->getNumWheels(); i++) {
                btWheelInfo &wheel = car->getRaycast()->getWheelInfo(i);
                wheel.m_suspensionStiffness = car->getSuspensionStiffness();
                wheel.m_wheelsDampingRelaxation = car->getSuspensionDamping();
                wheel.m_wheelsDampingCompression = car->getSuspensionCompression();
                wheel.m_frictionSlip = car->getWheelFriction();
                wheel.m_rollInfluence = car->getRollInfluence();
            }

            break;
    }
}

void Renderer::DrawMetadata(Entity *targetEntity) {
    ImGui::Begin("Engine Entity");
    ImGui::Text("%s", ToString(targetEntity->tag));
    ImGui::Text("%s", ToString(targetEntity->type));
    // Only display these if they're relevant
    if (targetEntity->parentTrackblockID != -1) {
        ImGui::Text("TrkBlk: %d", targetEntity->parentTrackblockID);
    }
    if (targetEntity->entityID != -1) {
        ImGui::Text("ID: %d", targetEntity->entityID);
    }
    ImGui::Separator();

    // Traverse the loader structures and print pretty with IMGUI
    switch (targetEntity->tag) {
        case NFSVer::NFS_3:
        case NFSVer::NFS_4:
            DrawNFS34Metadata(targetEntity);
            break;
        case NFSVer::UNKNOWN:
            ASSERT(false, "Unimplemented");
            break;
        case NFSVer::NFS_1:
            ASSERT(false, "Unimplemented");
            break;
        case NFSVer::NFS_2:
            break;
        case NFSVer::NFS_2_PS1:
            ASSERT(false, "Unimplemented");
            break;
        case NFSVer::NFS_2_SE:
            ASSERT(false, "Unimplemented");
            break;
        case NFSVer::NFS_3_PS1:
            break;
        case NFSVer::NFS_5:
            ASSERT(false, "Unimplemented");
            break;
    }
    ImGui::End();
}

void Renderer::SetCulling(bool toCull) {
    if (toCull) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void Renderer::DrawUI(ParamData *preferences, glm::vec3 worldPosition) {
    // Draw Logger UI
    logger->onScreenLog.Draw("ONFS Log");
    // Draw UI (Tactically)
    static float f = 0.0f;
    ImGui::Text("OpenNFS Engine");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::SliderFloat("Time Scale Factor", &preferences->timeScaleFactor, 0, 10);
    ImGui::Checkbox("Bullet Debug View", &preferences->physics_debug_view);
    ImGui::Checkbox("Classic Graphics", &preferences->use_classic_graphics);
    ImGui::Checkbox("Hermite Curve Cam", &preferences->attach_cam_to_hermite);
    ImGui::Checkbox("Car Cam", &preferences->attach_cam_to_car);
    std::stringstream world_position_string;
    ImGui::Text("X %f Y %f Z %f H: %f V: %f", worldPosition.x, worldPosition.y, worldPosition.z, mainCamera.horizontalAngle, mainCamera.verticalAngle);
    ImGui::Text("CarCam Yaw: %f Pitch: %f Distance: %f AAC: %f", mainCamera.yaw, mainCamera.pitch, mainCamera.distanceFromCar, mainCamera.angleAroundCar);
    ImGui::Text("Hermite Roll: %f Time: %f", mainCamera.roll, fmod(totalTime, (mainCamera.loopTime / 200)));
    ImGui::Text("Block ID: %d", closestBlockID);
    ImGui::Text("Frustrum Objects: %d", physicsEngine.numObjects);
    ImGui::Checkbox("Frustum Cull", &preferences->frustum_cull);
    ImGui::Checkbox("Raycast Viz", &preferences->draw_raycast);
    ImGui::Checkbox("AI Sim", &preferences->simulate_car);
    ImGui::Checkbox("Vroad Viz", &preferences->draw_vroad);
    ImGui::Checkbox("CAN Debug", &preferences->draw_can);

    if (ImGui::Button("Reset View")) {
        mainCamera.resetView();
    };
    ImGui::SameLine(0, -1.0f);
    if (ImGui::Button("Reset Car")) {
        car->resetCar(glm::vec3(track->track_blocks[0].center.x, track->track_blocks[0].center.y, track->track_blocks[0].center.z));
    };
    ImGui::NewLine();
    ImGui::SameLine(0, 0.0f);
    if (!preferences->use_nb_data)
        ImGui::SliderInt("Draw Dist", &preferences->blockDrawDistance, 0, track->nBlocks);
    ImGui::Checkbox("NBData", &preferences->use_nb_data);
    ImGui::NewLine();
    ImGui::ColorEdit3("Sky Colour", (float *) &preferences->clear_color); // Edit 3 floats representing a color
    //ImGui::SliderFloat3("NFS2 Rot Dbg", (float *) &preferences->nfs2_rotate, -M_PI, M_PI);

    ImGui::SliderFloat("Track Specular Damper", &preferences->trackSpecDamper, 0, 100);
    ImGui::SliderFloat("Track Specular Reflectivity", &preferences->trackSpecReflectivity, 0, 10);

    if (ImGui::TreeNode("Car Models")) {
        ImGui::Checkbox(car->car_body_model.m_name.c_str(), &car->car_body_model.enabled);
        ImGui::Checkbox(car->left_front_wheel_model.m_name.c_str(), &car->left_front_wheel_model.enabled);
        ImGui::Checkbox(car->left_rear_wheel_model.m_name.c_str(), &car->left_rear_wheel_model.enabled);
        ImGui::Checkbox(car->right_front_wheel_model.m_name.c_str(), &car->right_front_wheel_model.enabled);
        ImGui::Checkbox(car->right_rear_wheel_model.m_name.c_str(), &car->right_rear_wheel_model.enabled);
        ImGui::TreePop();
        if (ImGui::TreeNode("Misc Models")) {
            for (auto &mesh : car->misc_models) {
                ImGui::Checkbox(mesh.m_name.c_str(), &mesh.enabled);
            }
            ImGui::TreePop();
        }
    }

    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::DrawDebugCube(glm::vec3 position) {
    float lightSize = 0.5;
    glm::vec3 position_min = glm::vec3(position.x - lightSize, position.y - lightSize, position.z - lightSize);
    glm::vec3 position_max = glm::vec3(position.x + lightSize, position.y + lightSize, position.z + lightSize);
    btVector3 colour = btVector3(0, 0, 0);
    physicsEngine.mydebugdrawer.drawBox(Utils::glmToBullet(position_min), Utils::glmToBullet(position_max), colour);
}

std::vector<int> Renderer::CullTrackBlocks(glm::vec3 oldWorldPosition, glm::vec3 worldPosition, int blockDrawDistance, bool useNeighbourData) {
    std::vector<int> activeTrackBlockIds;

    // Basic Geometry Cull
    if ((oldWorldPosition.x != worldPosition.x) || (oldWorldPosition.z != worldPosition.z)) {
        cameraLight.position = worldPosition;
        float lowestDistanceSqr = FLT_MAX;
        //Primitive Draw distance
        for (auto &track_block :  track->track_blocks) {
            float distanceSqr = glm::length2(glm::distance(worldPosition, track_block.center));
            if (distanceSqr < lowestDistanceSqr) {
                closestBlockID = track_block.block_id;
                lowestDistanceSqr = distanceSqr;
            }
        }

        // If we have an NFS3 track loaded, use the provided neighbour data to work out which blocks to render
        if ((track->tag == NFS_3 || track->tag == NFS_4) && useNeighbourData) {
            for (int i = 0; i < 300; ++i) {
                if (boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[closestBlockID].nbdData[i].blk == -1) {
                    break;
                } else {
                    activeTrackBlockIds.emplace_back(boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[closestBlockID].nbdData[i].blk);
                }
            }
        } else {
            // Use a draw distance value to return closestBlock +- drawDistance inclusive blocks
            int wrapBlocks = 0;
            for (int block_Idx = closestBlockID - blockDrawDistance; block_Idx < closestBlockID + blockDrawDistance; ++block_Idx) {
                if (block_Idx < 0) {
                    int activeBlock = ((int) track->track_blocks.size() + (closestBlockID - blockDrawDistance)) + wrapBlocks++;
                    activeTrackBlockIds.emplace_back(activeBlock);
                } else {
                    activeTrackBlockIds.emplace_back(block_Idx % track->track_blocks.size());
                }
            }
        }
        oldWorldPosition = worldPosition;
    }
    // Render far to near
    return std::vector<int>(activeTrackBlockIds.rbegin(), activeTrackBlockIds.rend());
}

bool Renderer::DrawMenuBar() {
    bool assetChange = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Track")) {
            for (auto &installedNFS : installedNFSGames) {
                if (ImGui::BeginMenu(ToString(installedNFS.tag))) {
                    for (auto &track : installedNFS.tracks) {
                        if (ImGui::MenuItem(track.c_str())) {
                            loadedAssets.trackTag = installedNFS.tag;
                            loadedAssets.track = track;
                            assetChange = true;
                        }
                    }
                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Car")) {
            for (auto &installedNFS : installedNFSGames) {
                if (ImGui::BeginMenu(ToString(installedNFS.tag))) {
                    for (auto &car : installedNFS.cars) {
                        if (ImGui::MenuItem(car.c_str())) {
                            loadedAssets.carTag = installedNFS.tag;
                            loadedAssets.car = car;
                            assetChange = true;
                        }
                    }
                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    return assetChange;
}

Renderer::~Renderer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Renderer::NewFrame(ParamData *userParams) {
    glClearColor(userParams->clear_color.x, userParams->clear_color.y, userParams->clear_color.z, userParams->clear_color.w);
    glfwPollEvents();
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Detect a click on the 3D Window by detecting a click that isn't on ImGui
    userParams->window_active = userParams->window_active ? userParams->window_active : ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) && (!ImGui::GetIO().WantCaptureMouse));
    if (!userParams->window_active) {
        ImGui::GetIO().MouseDrawCursor = false;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
