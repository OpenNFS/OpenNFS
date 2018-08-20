//
// Created by SYSTEM on 19/07/2018.
//


#include "Renderer.h"


Renderer::Renderer(GLFWwindow *gl_window, const shared_ptr<ONFSTrack> &current_track, shared_ptr<Car> current_car) : carShader(current_car) {
    window = gl_window;
    track = current_track;
    car = current_car;

    mainCamera = Camera(glm::vec3(track->track_blocks[0].center.x / 10, track->track_blocks[0].center.y / 10, track->track_blocks[0].center.z / 10), 45.0f, 4.86f, -0.21f, window);
    mainCamera.generateSpline(track->track_blocks);

    cameraLight = Light(mainCamera.position, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), 1, 0, 0, 0, 0.f);

    // Generate the collision meshes
    physicsEngine.registerTrack(track);
    physicsEngine.registerVehicle(car);

    /*------- ImGui -------*/
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();
}

void Renderer::Render() {
    ParamData userParams;

    Light sun = Light(glm::vec3(10000, 150000, -10000), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);

    // Detect position change to trigger Cull code
    glm::vec3 oldWorldPosition(0, 0, 0);

    // Shit
    std::vector<Light> camlights;
    camlights.push_back(cameraLight);

    // Map of COL animated object to anim keyframe
    std::map<int, int> animMap;

    car->resetCar(glm::vec3(track->track_blocks[0].center.x / 10, track->track_blocks[0].center.y / 10, track->track_blocks[0].center.z / 10));

    bool entity_targeted = false;
    Entity *targetedEntity;

    if(!car->car_body_model.texture_ids.size()){
        carShader.load_tga_texture();
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(userParams.clear_color.x, userParams.clear_color.y, userParams.clear_color.z,
                     userParams.clear_color.w);
        NewFrame(userParams.window_active);

        if (userParams.attach_cam_to_hermite) {
            mainCamera.useSpline();
        }

        if (userParams.attach_cam_to_car) {
            // Compute MVP from keyboard and mouse, centered around a target car
            mainCamera.followCar(car, userParams.window_active, ImGui::GetIO());
        } else {
            // Compute the MVP matrix from keyboard and mouse input
            mainCamera.computeMatricesFromInputs(userParams.window_active, ImGui::GetIO());
        }

        glm::mat4 ProjectionMatrix = mainCamera.ProjectionMatrix;
        glm::mat4 ViewMatrix = mainCamera.ViewMatrix;
        glm::vec3 worldPosition = mainCamera.position;
        physicsEngine.mydebugdrawer.SetMatrices(ViewMatrix, ProjectionMatrix);

        //TODO: Refactor to controller class? AND USE SDL
        if (userParams.window_active && !ImGui::GetIO().MouseDown[1]) {
            car->applyAccelerationForce(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS,
                                        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
            car->applyBrakingForce(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
            car->applySteeringRight(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
            car->applySteeringLeft(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            // Go and find the Vroad Data to reset to
            if (track->tag == NFS_3) {
                car->resetCar(glm::vec3(track->track_blocks[closestBlockID].center.x / 10, track->track_blocks[closestBlockID].center.y / 10, track->track_blocks[closestBlockID].center.z / 10));
                /*if (boost::get<NFS3_4_DATA::TRACK>(track->trackData).trk[closestBlockID].nVRoad){
                    boost::get<NFS3_4_DATA::TRACK>(track->trackData).trk[closestBlockID].vroadData[0].x
                }*/
            }
        }

        // Step the physics simulation
        physicsEngine.stepSimulation(mainCamera.deltaTime);

        std::vector<int> activeTrackBlockIDs = CullTrackBlocks(oldWorldPosition, worldPosition, userParams.blockDrawDistance, userParams.use_nb_data);

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
                trackShader.loadMatrices(ProjectionMatrix, ViewMatrix, boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
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
                trackShader.loadMatrices(ProjectionMatrix, ViewMatrix, boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
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
                trackShader.loadMatrices(ProjectionMatrix, ViewMatrix, boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
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
                        boost::get<Track>(global_object.glMesh).position = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) * glm::vec3((col.object[global_object.entityID].animData[animMap[global_object.entityID]].pt.x / 65536.0) / 10, (col.object[global_object.entityID].animData[animMap[global_object.entityID]].pt.y / 65536.0) / 10, (col.object[global_object.entityID].animData[animMap[global_object.entityID]].pt.z / 65536.0) / 10);
                        boost::get<Track>(global_object.glMesh).orientation = glm::normalize(glm::quat(glm::vec3(-M_PI, -M_PI, 0))) * glm::normalize(glm::quat(-col.object[global_object.entityID].animData[animMap[global_object.entityID]].od1, col.object[global_object.entityID].animData[animMap[global_object.entityID]].od2, col.object[global_object.entityID].animData[animMap[global_object.entityID]].od3, col.object[global_object.entityID].animData[animMap[global_object.entityID]].od4));
                        animMap[global_object.entityID]++;
                    } else {
                        animMap[global_object.entityID] = 0;
                    }
                }
            }
            boost::get<Track>(global_object.glMesh).update();
            trackShader.loadMatrices(ProjectionMatrix, ViewMatrix, boost::get<Track>(global_object.glMesh).ModelMatrix);
            trackShader.loadSpecular(userParams.trackSpecDamper, userParams.trackSpecReflectivity);
            trackShader.loadLights(camlights);
            trackShader.bindTrackTextures(boost::get<Track>(global_object.glMesh), track->texture_gl_mappings);
            trackShader.setClassic(userParams.use_classic_graphics);
            boost::get<Track>(global_object.glMesh).render();
        }
        trackShader.unbind();

        //SetCulling(true);
        // Render the Car
        carShader.use();
        // Clean this up
        if(car->car_body_model.texture_ids.size()){
            carShader.setMultiTextured();
            carShader.bindCarTextures(car->car_body_model, car->car_texture_gl_mappings);
        } else {
            carShader.loadCarTexture();
        }
        carShader.loadMatrices(ProjectionMatrix, ViewMatrix, car->car_body_model.ModelMatrix);
        carShader.loadSpecular(car->car_body_model.specularDamper, car->car_body_model.specularReflectivity, car->car_body_model.envReflectivity);
        carShader.loadCarColor(glm::vec3(1, 1, 1));
        carShader.loadLight(cameraLight);
        car->car_body_model.render();

        if(car->left_front_wheel_model.texture_ids.size()){
            carShader.setMultiTextured();
            carShader.bindCarTextures(car->left_front_wheel_model, car->car_texture_gl_mappings);
        } else {
            carShader.loadCarTexture();
        }
        carShader.loadMatrices(ProjectionMatrix, ViewMatrix, car->left_front_wheel_model.ModelMatrix);
        carShader.loadSpecular(car->left_front_wheel_model.specularDamper, 0, 0);
        carShader.loadCarColor(glm::vec3(1, 1, 1));
        carShader.loadLight(cameraLight);
        car->left_front_wheel_model.render();

        if(car->left_rear_wheel_model.texture_ids.size()){
            carShader.setMultiTextured();
            carShader.bindCarTextures(car->left_rear_wheel_model, car->car_texture_gl_mappings);
        } else {
            carShader.loadCarTexture();
        }
        carShader.loadMatrices(ProjectionMatrix, ViewMatrix, car->left_rear_wheel_model.ModelMatrix);
        carShader.loadSpecular(car->left_rear_wheel_model.specularDamper, 0, 0);
        carShader.loadCarColor(glm::vec3(1, 1, 1));
        carShader.loadLight(cameraLight);
        car->left_rear_wheel_model.render();

        if(car->right_front_wheel_model.texture_ids.size()){
            carShader.setMultiTextured();
            carShader.bindCarTextures(car->right_front_wheel_model, car->car_texture_gl_mappings);
        } else {
            carShader.loadCarTexture();
        }
        carShader.loadMatrices(ProjectionMatrix, ViewMatrix, car->right_front_wheel_model.ModelMatrix);
        carShader.loadSpecular(car->right_front_wheel_model.specularDamper, 0, 0);
        carShader.loadCarColor(glm::vec3(1, 1, 1));
        carShader.loadLight(cameraLight);
        car->right_front_wheel_model.render();

        if(car->right_rear_wheel_model.texture_ids.size()){
            carShader.setMultiTextured();
            carShader.bindCarTextures(car->right_rear_wheel_model, car->car_texture_gl_mappings);
        } else {
            carShader.loadCarTexture();
        }
        carShader.loadMatrices(ProjectionMatrix, ViewMatrix, car->right_rear_wheel_model.ModelMatrix);
        carShader.loadSpecular(car->right_rear_wheel_model.specularDamper, 0, 0);
        carShader.loadCarColor(glm::vec3(1, 1, 1));
        carShader.loadLight(cameraLight);
        car->right_rear_wheel_model.render();

        for (auto &misc_model : car->misc_models) {
            // Clean this up
            if(misc_model.texture_ids.size()){
                carShader.setMultiTextured();
                carShader.bindCarTextures(misc_model, car->car_texture_gl_mappings);
            } else {
                carShader.loadCarTexture();
            }
            carShader.loadMatrices(ProjectionMatrix, ViewMatrix, misc_model.ModelMatrix);
            carShader.loadSpecular(misc_model.specularDamper, 0, 0);
            carShader.loadCarColor(glm::vec3(1, 1, 1));
            carShader.loadLight(cameraLight);
            misc_model.render();
        }
        carShader.unbind();
        //SetCulling(false);

        for (auto &track_block_id : activeTrackBlockIDs) {
            billboardShader.use();
            // Render the lights far to near
            for (auto &light_entity : std::vector<Entity>(track->track_blocks[track_block_id].lights.rbegin(), track->track_blocks[track_block_id].lights.rend())) {
                boost::get<Light>(light_entity.glMesh).update();
                billboardShader.loadMatrices(ProjectionMatrix, ViewMatrix, boost::get<Light>(light_entity.glMesh).ModelMatrix);
                billboardShader.loadLight(boost::get<Light>(light_entity.glMesh));
                boost::get<Light>(light_entity.glMesh).render();
            }
            billboardShader.unbind();
        }

        if (ImGui::GetIO().MouseReleased[0] & userParams.window_active) {
            targetedEntity = CheckForPicking(ViewMatrix, ProjectionMatrix, &entity_targeted);
        }

        if (entity_targeted) {
            DrawMetadata(targetedEntity);
        }

        if (userParams.physics_debug_view)
            physicsEngine.getDynamicsWorld()->debugDrawWorld();

        DrawUI(&userParams, worldPosition);
        glfwSwapBuffers(window);
    }
}

Entity *Renderer::CheckForPicking(glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, bool *entity_targeted) {
    glm::vec3 out_origin;
    glm::vec3 out_direction;
    ScreenPosToWorldRay(1024 / 2, 768 / 2, 1024, 768, ViewMatrix, ProjectionMatrix, out_origin, out_direction);
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
            ImGui::Text("Type: %d", targetLight->type);
            ImGui::Text("Unknowns: ");
            ImGui::Text("[1]: %d", targetLight->unknown1);
            ImGui::Text("[2]: %d", targetLight->unknown2);
            ImGui::Text("[3]: %d", targetLight->unknown3);
            ImGui::Text("[4]: %d", targetLight->unknown4);
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
            ImGui::SliderFloat("Friction.", &targetCar->wheelFriction, 0, 1000000);
            ImGui::SliderFloat("Roll Infl.", &targetCar->rollInfluence, 0, 0.5);
            ImGui::SliderFloat("Steer Incr.", &targetCar->steeringIncrement, 0, 0.1);
            ImGui::SliderFloat("Steer Clamp", &targetCar->steeringClamp, 0, 0.5);
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
        case NFSVer::NFS_4:
            DrawNFS34Metadata(targetEntity);
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
    // Draw UI (Tactically)
    static float f = 0.0f;
    DrawMenuBar();
    ImGui::Text("OpenNFS Engine");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Checkbox("Bullet Debug View", &preferences->physics_debug_view);
    ImGui::Checkbox("Classic Graphics", &preferences->use_classic_graphics);
    ImGui::Checkbox("Hermite Curve Cam", &preferences->attach_cam_to_hermite);
    ImGui::Checkbox("Car Cam", &preferences->attach_cam_to_car);
    std::stringstream world_position_string;
    ImGui::Text("X %f Y %f Z %f H: %f V: %f", worldPosition.x, worldPosition.y, worldPosition.z, mainCamera.horizontalAngle, mainCamera.verticalAngle);
    ImGui::Text("CarCam Yaw: %f Pitch: %f Distance: %f AAC: %f", mainCamera.yaw, mainCamera.pitch, mainCamera.distanceFromCar, mainCamera.angleAroundCar);
    ImGui::Text(("Block ID: " + std::to_string(closestBlockID)).c_str());

    if (ImGui::Button("Reset View")) {
        mainCamera.resetView();
    };
    ImGui::SameLine(0, -1.0f);
    if (ImGui::Button("Reset Car")) {
        car->resetCar(glm::vec3(track->track_blocks[0].center.x / 10, track->track_blocks[0].center.y / 10, track->track_blocks[0].center.z / 10));
    };
    ImGui::NewLine();
    ImGui::SameLine(0, 0.0f);
    if (!preferences->use_nb_data)
        ImGui::SliderInt("Draw Dist", &preferences->blockDrawDistance, 0, track->nBlocks);
    ImGui::Checkbox("NBData", &preferences->use_nb_data);
    ImGui::NewLine();
    ImGui::ColorEdit3("Clear Colour", (float *) &preferences->clear_color); // Edit 3 floats representing a color
    ImGui::ColorEdit3("Testing Light Colour", (float *) &preferences->test_light_color);
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
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::DrawDebugCube(glm::vec3 position) {
    float lightSize = 0.5;
    glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
    glm::vec3 position_min =
            orientation * glm::vec3(position.x - lightSize, position.y - lightSize, position.z - lightSize);
    glm::vec3 position_max =
            orientation * glm::vec3(position.x + lightSize, position.y + lightSize, position.z + lightSize);
    btVector3 colour = btVector3(0, 0, 0);
    physicsEngine.mydebugdrawer.drawBox(btVector3(position_min.x, position_min.y, position_min.z), btVector3(position_max.x, position_max.y, position_max.z), colour);
}

std::vector<int> Renderer::CullTrackBlocks(glm::vec3 oldWorldPosition, glm::vec3 worldPosition, int blockDrawDistance,
                                           bool useNeighbourData) {
    std::vector<int> activeTrackBlockIds;

    // Basic Geometry Cull
    if ((oldWorldPosition.x != worldPosition.x) && (oldWorldPosition.z != worldPosition.z)) {
        cameraLight.position = worldPosition;
        float lowestDistanceSqr = FLT_MAX;
        //Primitive Draw distance
        for (auto &track_block :  track->track_blocks) {
            glm::vec3 position = glm::vec3(track_block.center.x / 10, track_block.center.y / 10, track_block.center.z / 10);
            float distanceSqr = glm::length2(glm::distance(worldPosition, position));
            if (distanceSqr < lowestDistanceSqr) {
                closestBlockID = track_block.block_id;
                lowestDistanceSqr = distanceSqr;
            }
        }

        // If we have an NFS3 track loaded, use the provided neighbour data to work out which blocks to render
        if (track->tag == NFS_3 && useNeighbourData) {
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
            for (int block_Idx = closestBlockID - blockDrawDistance;
                 block_Idx < closestBlockID + blockDrawDistance; ++block_Idx) {
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

void Renderer::DrawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
            if (ImGui::BeginMenu("Open Recent")) {
                ImGui::MenuItem("fish_hat.c");
                ImGui::MenuItem("fish_hat.inl");
                ImGui::MenuItem("fish_hat.h");
                if (ImGui::BeginMenu("More..")) {
                    ImGui::MenuItem("Hello");
                    ImGui::MenuItem("Sailor");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Options")) {
                static bool enabled = true;
                ImGui::MenuItem("Enabled", "", &enabled);
                ImGui::BeginChild("child", ImVec2(0, 60), true);
                for (int i = 0; i < 10; i++)
                    ImGui::Text("Scrolling Text %d", i);
                ImGui::EndChild();
                static float f = 0.5f;
                static int n = 0;
                static bool b = true;
                ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
                ImGui::InputFloat("Input", &f, 0.1f);
                ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
                ImGui::Checkbox("Check", &b);
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Quit", "Alt+F4")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

Renderer::~Renderer() {
    // Cleanup VBOs and shaders
    carShader.cleanup();
    trackShader.cleanup();
    ImGui_ImplGlfwGL3_Shutdown();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

void Renderer::NewFrame(bool &window_active) {
    glfwPollEvents();
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Detect a click on the 3D Window by detecting a click that isn't on ImGui
    window_active = window_active ? window_active : (
            (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) && (!ImGui::GetIO().WantCaptureMouse));
    if (!window_active) {
        ImGui::GetIO().MouseDrawCursor = false;
    }
    ImGui_ImplGlfwGL3_NewFrame();
}
