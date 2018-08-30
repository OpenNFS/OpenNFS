//
// Created by Amrik Sadhra on 19/07/2018.
//


#include "Renderer.h"


Renderer::Renderer(GLFWwindow *gl_window, const std::vector<NeedForSpeed> &installedNFS, const shared_ptr<ONFSTrack> &current_track, shared_ptr<Car> current_car) : carRenderer(current_car), trackRenderer(current_track) {
    window = gl_window;
    installedNFSGames = installedNFS;
    track = current_track;
    car = current_car;

    loadedAssets.carTag = car->tag;
    loadedAssets.car = car->name;
    loadedAssets.trackTag = track->tag;
    loadedAssets.track = track->name;

    mainCamera = Camera(glm::vec3(track->track_blocks[0].center.x / 10, track->track_blocks[0].center.y / 10, track->track_blocks[0].center.z / 10), 45.0f, 4.86f, -0.21f, window);
    mainCamera.generateSpline(track->track_blocks);
    cameraLight = Light(mainCamera.position, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), 1, 0, 0, 0, 0.f);

    // Generate the collision meshes
    physicsEngine.registerTrack(track);
    physicsEngine.registerVehicle(car);

    /*------- ImGui -------*/
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();
}

AssetData Renderer::Render() {
    ParamData userParams;
    if((track->tag == NFS_2_SE || track->tag == NFS_2 || track->tag == NFS_3_PS1)){
        userParams.use_nb_data = false;
    }

    Light sun = Light(glm::vec3(10000, 150000, -10000), glm::vec4(255, 255, 255, 255), 0, 0, 0, 0, 0);

    // Detect position change to trigger Cull code
    glm::vec3 oldWorldPosition(0, 0, 0);

    car->resetCar(glm::vec3(track->track_blocks[0].center.x, track->track_blocks[0].center.y, track->track_blocks[0].center.z));

    bool entity_targeted = false;
    Entity *targetedEntity;

    bool newAssetSelected = false;

    while (!glfwWindowShouldClose(window)) {
        NewFrame(&userParams);

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

        physicsEngine.mydebugdrawer.SetMatrices(mainCamera.ViewMatrix, mainCamera.ProjectionMatrix);

        //TODO: Refactor to controller class? AND USE SDL
        if (userParams.window_active && !ImGui::GetIO().MouseDown[1]) {
            car->applyAccelerationForce(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS, glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
            car->applyBrakingForce(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
            car->applySteeringRight(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
            car->applySteeringLeft(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            // Go and find the Vroad Data to reset to
            if (track->tag == NFS_3) {
                car->resetCar(glm::vec3(track->track_blocks[closestBlockID].center.x, (track->track_blocks[closestBlockID].center.y)+0.2, track->track_blocks[closestBlockID].center.z));
                /*if (boost::get<NFS3_4_DATA::TRACK>(track->trackData).trk[closestBlockID].nVRoad){
                    boost::get<NFS3_4_DATA::TRACK>(track->trackData).trk[closestBlockID].vroadData[0].x
                }*/
            } else if(track->tag == NFS_2_SE || track->tag == NFS_2 || track->tag == NFS_3_PS1){
                car->resetCar(glm::vec3(track->track_blocks[closestBlockID].center.x, (track->track_blocks[closestBlockID].center.y+0.2), track->track_blocks[closestBlockID].center.z));
            }
        }

        // Step the physics simulation
        physicsEngine.stepSimulation(mainCamera.deltaTime);

        std::vector<int> activeTrackBlockIDs = CullTrackBlocks(oldWorldPosition, userParams.attach_cam_to_car ? car->car_body_model.position : mainCamera.position, userParams.blockDrawDistance, userParams.use_nb_data);
        trackRenderer.renderTrack(mainCamera, cameraLight, activeTrackBlockIDs, userParams);

        //SetCulling(true);
        carRenderer.render(mainCamera, cameraLight);
        //SetCulling(false);

        trackRenderer.renderLights(mainCamera, activeTrackBlockIDs);

        if (ImGui::GetIO().MouseReleased[0] & userParams.window_active) {
            targetedEntity = CheckForPicking(mainCamera.ViewMatrix, mainCamera.ProjectionMatrix, &entity_targeted);
        }

        if (entity_targeted) {
            DrawMetadata(targetedEntity);
        }

        if (userParams.physics_debug_view)
            physicsEngine.getDynamicsWorld()->debugDrawWorld();

        if(DrawMenuBar()){
            newAssetSelected = true;
        };
        DrawUI(&userParams, mainCamera.position);
        glfwSwapBuffers(window);

        if(newAssetSelected) break;
    }
    if(newAssetSelected){
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
    ImGui::ColorEdit3("Sky Colour", (float *) &preferences->clear_color); // Edit 3 floats representing a color
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
    if ((oldWorldPosition.x != worldPosition.x) && (oldWorldPosition.z != worldPosition.z)) {
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

bool Renderer::DrawMenuBar() {
    bool assetChange = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Track")) {
            for(auto &installedNFS : installedNFSGames){
                if (ImGui::BeginMenu(ToString(installedNFS.tag))) {
                    for(auto &track : installedNFS.tracks){
                        if(ImGui::MenuItem(track.c_str())){
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
            for(auto &installedNFS : installedNFSGames){
                if (ImGui::BeginMenu(ToString(installedNFS.tag))) {
                    for(auto &car : installedNFS.cars){
                        if(ImGui::MenuItem(car.c_str())){
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
    /*if (ImGui::BeginMainMenuBar()) {
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
    }*/
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
