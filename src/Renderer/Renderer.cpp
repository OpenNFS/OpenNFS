//
// Created by Amrik Sadhra on 19/07/2018.
//


#include "Renderer.h"

Renderer::Renderer(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger,
                   const std::vector<NeedForSpeed> &installedNFS, const std::shared_ptr<ONFSTrack> &currentTrack,
                   std::shared_ptr<Car> &currentCar) : trackRenderer(currentTrack),
                                                   skyRenderer(currentTrack), shadowMapRenderer(currentTrack),
                                                   logger(onfsLogger), installedNFSGames(installedNFS),
                                                   window(glWindow), track(currentTrack) {
    InitialiseIMGUI();
    InitGlobalLights();
    // Skip CAN animation if PS1 track loaded
    cameraAnimationPlayed = track->tag == NFS_3_PS1;
    LOG(DEBUG) << "Renderer Initialised";
}

GLFWwindow *InitOpenGL(int resolutionX, int resolutionY, const std::string &windowName) {
    // Initialise GLFW
    ASSERT(glfwInit(), "GLFW Init failed.\n");
    glfwSetErrorCallback(&Renderer::GlfwError);

    // TODO: Disable MSAA for now until texture array adds padding
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    // TODO: If we fail to create a GL context on Windows, fall back to not requesting any (Keiiko Bug #1)
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif

    GLFWwindow *window = glfwCreateWindow(resolutionX, resolutionY, windowName.c_str(), nullptr, nullptr);

    if (window == nullptr) {
        LOG(WARNING) << "Failed to create a GLFW window.";
        getchar();
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);

    glfwSetWindowSizeCallback(window, Renderer::WindowSizeCallback);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile

    if (glewInit() != GLEW_OK) {
        LOG(WARNING) << "Failed to initialize GLEW";
        getchar();
        glfwTerminate();
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Set the mouse at the center of the screen
    glfwPollEvents();

    // Dark blue background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    GLint texture_units, max_array_texture_layers;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_array_texture_layers);
    LOG(DEBUG) << "Max Texture Units: " << texture_units;
    LOG(DEBUG) << "Max Array Texture Layers: " << max_array_texture_layers;
    LOG(DEBUG) << "OpenGL Initialisation successful";

    //glfwSwapInterval(1);

    return window;
}

void Renderer::InitGlobalLights() {
    // Set some light parameters
    moon.attenuation.x = sun.attenuation.x = 0.710f;
    moon.attenuation.y = sun.attenuation.y = 0;
    moon.attenuation.z = sun.attenuation.z = 0;
}

bool Renderer::UpdateGlobalLights(ParamData &userParams) {
    // Move the Sun, and update the position it's looking (for test)
    sun.position = sun.position * glm::normalize(glm::quat(glm::vec3(userParams.timeScaleFactor * 0.001f, 0, 0)));
    moon.position = moon.position * glm::normalize(glm::quat(glm::vec3(userParams.timeScaleFactor * 0.001f, 0, 0)));

    sun.lookAt = moon.lookAt = track->trackBlocks[closestBlockID].center;
    sun.update();
    moon.update();

    return sun.position.y <= 0;
}

bool Renderer::Render(float totalTime, float deltaTime, Camera &camera, ParamData &userParams, AssetData &loadedAssets, std::shared_ptr<Car> &playerCar, std::vector<CarAgent> racers, PhysicsEngine &physicsEngine) {
    bool newAssetSelected = false;

    // Hot reload shaders
    UpdateShaders();
    NewFrame(userParams);

    // TODO: Extract camera back up to RaceSession class if can resolve mouse lag when calling this code before NewFrame
    // Play the original camera animation
    if (!cameraAnimationPlayed) {
        cameraAnimationPlayed = camera.playAnimation(playerCar->carBodyModel.position);
    } else if (userParams.attachCamToHermite) {
        camera.useSpline(totalTime);
    } else if (userParams.attachCamToCar) {
        // Compute MVP from keyboard and mouse, centered around a target car
        camera.followCar(playerCar, userParams.windowActive);
    } else {
        // Compute the MVP matrix from keyboard and mouse input
        camera.computeMatricesFromInputs(userParams.windowActive, deltaTime);
    }

    std::vector<int> activeTrackBlockIDs;
    if (userParams.frustumCull) {
        physicsEngine.updateFrustrum(camera.ViewMatrix);
        // Iterate through visible entity list, based on frustum intersection
        for (int i = 0; i < physicsEngine.m_objectsInFrustum.size(); ++i) {
            auto *visibleEntity = static_cast<Entity *>(physicsEngine.m_objectsInFrustum[i]->getUserPointer());
            if (visibleEntity->type == ROAD) {
                activeTrackBlockIDs.emplace_back(visibleEntity->parentTrackblockID);
            }
        }
    } else {
        physicsEngine.destroyGhostObject();
        activeTrackBlockIDs = CullTrackBlocks(
                userParams.attachCamToHermite ? camera.position : userParams.attachCamToCar
                                                                      ? playerCar->carBodyModel.position
                                                                      : camera.position,
                userParams.blockDrawDistance, userParams.useNbData);
    }

    // If Sun moving below Horizon, change 'Sun' to 'Moon' and flip some state so we know to drop ambient in TrackShader
    bool nightTime = UpdateGlobalLights(userParams);
    float ambientLightFactor = nightTime ? 0.2f : 0.5f;

    shadowMapRenderer.renderShadowMap(userParams.nearPlane, userParams.farPlane, nightTime ? moon.ViewMatrix : sun.ViewMatrix, activeTrackBlockIDs, playerCar, racers);
    skyRenderer.renderSky(camera, sun, userParams, totalTime);
    trackRenderer.renderTrack(playerCar, camera, nightTime ? moon : sun, activeTrackBlockIDs, userParams, shadowMapRenderer.depthTextureID, shadowMapRenderer.lightSpaceMatrix, ambientLightFactor);
    trackRenderer.renderLights(camera, activeTrackBlockIDs);
    //menuRenderer.render();

    // Render the Car and racers
    // Get lights that will contribute to car body (currentBlock, a few blocks forward, and a few back (NBData would give weird results, as NBData blocks aren't generally adjacent))
    // Should use NFS3/4 Shading data too as a fake light
    std::vector<Light> carBodyContributingLights = trackRenderer.trackLightMap[closestBlockID];
    carBodyContributingLights.emplace_back(nightTime ? moon : sun);
    if (playerCar->tag == NFS_3 || playerCar->tag == NFS_4) SetCulling(true);
    carRenderer.render(playerCar, camera, carBodyContributingLights);
    for(auto &racer : racers){
        if (racer.car->tag == NFS_3 || racer.car->tag == NFS_4) SetCulling(true);
        carRenderer.render(racer.car, camera, carBodyContributingLights);
    }
    SetCulling(false);

    if (userParams.drawRaycast) {
        DrawCarRaycasts(playerCar, physicsEngine);
    }

    if (userParams.drawCAN) {
        DrawCameraAnimation(camera, physicsEngine);
    }

    if (userParams.drawVroad) {
        DrawVroad(physicsEngine);
    }

    if (ImGui::GetIO().MouseReleased[0] & userParams.windowActive) {
        targetedEntity = physicsEngine.checkForPicking(camera.ViewMatrix, camera.ProjectionMatrix, &entityTargeted);
    }

    if (entityTargeted) {
        DrawMetadata(targetedEntity);
    }

    if (userParams.physicsDebugView) {
        physicsEngine.getDynamicsWorld()->debugDrawWorld();
    }

    if (DrawMenuBar(loadedAssets)) {
        newAssetSelected = true;
    };

    DrawUI(userParams, camera, playerCar);
    glfwSwapBuffers(window);

    return newAssetSelected;
}

void Renderer::InitialiseIMGUI() {
    /*------- ImGui -------*/
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    std::string imgui_gl_version = "#version " + ONFS_GL_VERSION;
    ImGui_ImplOpenGL3_Init(imgui_gl_version.c_str());
    ImGui::StyleColorsDark();
}

void Renderer::DrawNFS34Metadata(Entity *targetEntity) {
    switch (targetEntity->type) {
        case EntityType::VROAD:
            break;
        case EntityType::OBJ_POLY:
            break;
        case EntityType::GLOBAL:
            break;
        case EntityType::LANE:
            break;
        case EntityType::LIGHT: {
            Light *targetLight = &boost::get<Light>(targetEntity->glMesh);
            ImVec4 lightColour(targetLight->colour.x, targetLight->colour.y, targetLight->colour.z,
                               targetLight->colour.w);
            ImVec4 lightAttenuation(targetLight->attenuation.x, targetLight->attenuation.y, targetLight->attenuation.z,
                                    0.0f);
            // Colour, type, attenuation, position and NFS unknowns
            ImGui::ColorEdit4("Light Colour", (float *) &lightColour); // Edit 3 floats representing a color
            targetLight->colour = glm::vec4(lightColour.x, lightColour.y, lightColour.z, lightColour.w);
            ImGui::SliderFloat3("Attenuation (A, B, C)", (float *) &lightAttenuation, 0, 10.0f);
            targetLight->attenuation = glm::vec3(lightAttenuation.x, lightAttenuation.y, lightAttenuation.z);
            ImGui::Text("x: %f y: %f z: %f ", targetLight->position.x, targetLight->position.y,
                        targetLight->position.z);
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
            ImGui::Text("%s Supported Colours:", targetCar->name.c_str());
            for(auto &carColour : targetCar->data.colours){
                ImVec4 carColourIm(carColour.colour.x, carColour.colour.y, carColour.colour.z, 0);
                ImGui::ColorEdit4(carColour.colourName.c_str(), (float *) &carColourIm); // Edit 3 floats representing a color
            }
            ImGui::Text("Ray Distances U: %f F: %f R: %f L: %f", targetCar->upDistance, targetCar->rangefinders[Car::FORWARD_RAY], targetCar->rangefinders[Car::RIGHT_RAY], targetCar->rangefinders[Car::LEFT_RAY]);
            ImGui::Text("Speed %f", targetCar->m_vehicle->getCurrentSpeedKmHour() / 10.f);
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
            /*ImGui::ColorEdit3("Car Colour", (float *) &userParams.car_color);
            ImGui::SliderFloat("Car Specular Damper", &userParams.carSpecDamper, 0, 100);
            ImGui::SliderFloat("Car Specular Reflectivity", &userParams.carSpecReflectivity, 0, 10);*/
            ImGui::Text("Roll (deg) x: %f y: %f z: %f",
                        glm::eulerAngles(targetCar->carBodyModel.orientation).x * 180 / SIMD_PI,
                        glm::eulerAngles(targetCar->carBodyModel.orientation).y * 180 / SIMD_PI,
                        glm::eulerAngles(targetCar->carBodyModel.orientation).z * 180 / SIMD_PI);

            // TODO: Only do this on a change
            for (int i = 0; i < targetCar->getRaycast()->getNumWheels(); i++) {
                btWheelInfo &wheel = targetCar->getRaycast()->getWheelInfo(i);
                wheel.m_suspensionStiffness = targetCar->getSuspensionStiffness();
                wheel.m_wheelsDampingRelaxation = targetCar->getSuspensionDamping();
                wheel.m_wheelsDampingCompression = targetCar->getSuspensionCompression();
                wheel.m_frictionSlip = targetCar->getWheelFriction();
                wheel.m_rollInfluence = targetCar->getRollInfluence();
            }

            break;
    }
    ImGui::Text("Object Flags: %d", targetEntity->flags);
    ImGui::Text("Collideable: %s", targetEntity->collideable ? "Yes" : "No");
    ImGui::Text("Dynamic: %s", targetEntity->dynamic ? "Yes" : "No");
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
            //ASSERT(false, "Unimplemented");
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

void Renderer::DrawUI(ParamData &userParams, Camera &camera, std::shared_ptr<Car> &playerCar) {
    // Draw Shadow Map
    ImGui::Begin("Shadow Map");
    ImGui::Image((ImTextureID) shadowMapRenderer.depthTextureID, ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, -1));
    ImGui::SliderFloat("Near Plane", &userParams.nearPlane, 0, 300);
    ImGui::SliderFloat("Far Plane", &userParams.farPlane, 0, 300);
    ImGui::End();
    // Draw Logger UI
    logger->onScreenLog.Draw("ONFS Log");
    // Draw UI (Tactically)
    ImGui::Text("OpenNFS Engine");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::SliderFloat("Time Scale Factor", &userParams.timeScaleFactor, 0, 10);
    ImGui::Checkbox("Bullet Debug View", &userParams.physicsDebugView);
    ImGui::Checkbox("Classic Graphics", &userParams.useClassicGraphics);
    ImGui::Checkbox("Hermite Curve Cam", &userParams.attachCamToHermite);
    ImGui::Checkbox("Car Cam", &userParams.attachCamToCar);
    std::stringstream world_position_string;
    ImGui::Text("X %f Y %f Z %f", camera.position.x, camera.position.y, camera.position.z);
    ImGui::Text("CarCam Yaw: %f Pitch: %f Distance: %f AAC: %f", camera.yaw, camera.pitch, camera.distanceFromCar, camera.angleAroundCar);
    ImGui::Text("Hermite Roll: %f", camera.roll);
    ImGui::Text("Block ID: %d", closestBlockID);
    ImGui::Text("Vroad ID: %d", CarAgent::getClosestVroad(playerCar, track));
    // ImGui::Text("Frustrum Objects: %d", physicsEngine.numObjects);
    ImGui::Checkbox("Frustum Cull", &userParams.frustumCull);
    ImGui::Checkbox("Raycast Viz", &userParams.drawRaycast);
    ImGui::Checkbox("AI Sim", &userParams.simulateCars);
    ImGui::Checkbox("Vroad Viz", &userParams.drawVroad);
    ImGui::Checkbox("CAN Debug", &userParams.drawCAN);

    if (ImGui::Button("Reset View")) {
        camera.resetView();
    };
    ImGui::SameLine(0, -1.0f);
    if (ImGui::Button("Reset Car to Start")) {
        CarAgent::resetToVroad(0, 0, 0.f, track, playerCar);
    };
    ImGui::NewLine();
    ImGui::SameLine(0, 0.0f);
    if (!userParams.useNbData)
        ImGui::SliderInt("Draw Dist", &userParams.blockDrawDistance, 0, track->nBlocks/2);
    ImGui::Checkbox("NBData", &userParams.useNbData);
    ImGui::NewLine();
    ImGui::ColorEdit3("Sun Atten", (float *) &userParams.sunAttenuation); // Edit 3 floats representing a color
    //ImGui::SliderFloat3("NFS2 Rot Dbg", (float *) &userParams.nfs2_rotate, -M_PI, M_PI);

    ImGui::SliderFloat("Track Specular Damper", &userParams.trackSpecDamper, 0, 100);
    ImGui::SliderFloat("Track Specular Reflectivity", &userParams.trackSpecReflectivity, 0, 10);

    if (ImGui::TreeNode("Car Models")) {
        ImGui::Checkbox(playerCar->carBodyModel.m_name.c_str(), &playerCar->carBodyModel.enabled);
        ImGui::Checkbox(playerCar->leftFrontWheelModel.m_name.c_str(), &playerCar->leftFrontWheelModel.enabled);
        ImGui::Checkbox(playerCar->leftRearWheelModel.m_name.c_str(), &playerCar->leftRearWheelModel.enabled);
        ImGui::Checkbox(playerCar->rightFrontWheelModel.m_name.c_str(), &playerCar->rightFrontWheelModel.enabled);
        ImGui::Checkbox(playerCar->rightRearWheelModel.m_name.c_str(), &playerCar->rightRearWheelModel.enabled);
        ImGui::TreePop();
        if (ImGui::TreeNode("Misc Models")) {
            for (auto &mesh : playerCar->miscModels) {
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

void Renderer::DrawDebugCube(PhysicsEngine &physicsEngine, glm::vec3 position) {
    float lightSize = 0.5;
    glm::vec3 position_min = glm::vec3(position.x - lightSize, position.y - lightSize, position.z - lightSize);
    glm::vec3 position_max = glm::vec3(position.x + lightSize, position.y + lightSize, position.z + lightSize);
    btVector3 colour = btVector3(0, 0, 0);
    physicsEngine.mydebugdrawer.drawBox(Utils::glmToBullet(position_min), Utils::glmToBullet(position_max), colour);
}

// Basic Geometry Cull
std::vector<int> Renderer::CullTrackBlocks(glm::vec3 worldPosition, int blockDrawDistance, bool useNeighbourData) {
    std::vector<int> activeTrackBlockIds;
    float lowestDistance = FLT_MAX;

    //Primitive Draw distance
    for (auto &track_block :  track->trackBlocks) {
        float distance = glm::distance(worldPosition, track_block.center);
        if (distance < lowestDistance) {
            closestBlockID = track_block.blockId;
            lowestDistance = distance;
        }
    }
    // If we have an NFS3 track loaded, use the provided neighbour data to work out which blocks to render
    if ((track->tag == NFS_3 || track->tag == NFS_4) && useNeighbourData) {
        for (int i = 0; i < 300; ++i) {
            if (boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[closestBlockID].nbdData[i].blk ==
                -1) {
                break;
            } else {
                activeTrackBlockIds.emplace_back(boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(
                        track->trackData)->trk[closestBlockID].nbdData[i].blk);
            }
        }
    } else {
        // Use a draw distance value to return closestBlock +- drawDistance inclusive blocks
        for (int block_Idx = closestBlockID - blockDrawDistance;
             block_Idx < closestBlockID + blockDrawDistance; ++block_Idx) {
            int activeBlock = block_Idx < 0 ? ((int) track->trackBlocks.size() + block_Idx) : (block_Idx %
                                                                                               (int) track->trackBlocks.size());
            activeTrackBlockIds.emplace_back(activeBlock);
        }
    }

    // Render far to near
    return std::vector<int>(activeTrackBlockIds.rbegin(), activeTrackBlockIds.rend());
}

bool Renderer::DrawMenuBar(AssetData &loadedAssets) {
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
    // Fixes bug on next render creation when install_callbacks set to true for ImGui
    glfwSetMouseButtonCallback(window, nullptr);
    glfwSetScrollCallback(window, nullptr);
    glfwSetKeyCallback(window, nullptr);
    glfwSetCharCallback(window, nullptr);
}

void Renderer::NewFrame(ParamData &userParams) {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glfwPollEvents();
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Detect a click on the 3D Window by detecting a click that isn't on ImGui
    userParams.windowActive = userParams.windowActive ? userParams.windowActive : (
            (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) && (!ImGui::GetIO().WantCaptureMouse));
    if (!userParams.windowActive) {
        ImGui::GetIO().MouseDrawCursor = false;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Renderer::DrawCarRaycasts(const std::shared_ptr<Car> &car, PhysicsEngine &physicsEngine) {
    glm::vec3 carBodyPosition = car->carBodyModel.position;

    for(uint8_t rangeIdx = 0; rangeIdx < Car::kNumRangefinders; ++rangeIdx){
        physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(carBodyPosition),
                                             Utils::glmToBullet(car->castPositions[rangeIdx]),
                                             btVector3(2.0f * (Car::kFarDistance - car->rangefinders[rangeIdx]), 2.0f * (car->rangefinders[rangeIdx]), 0));
    }
    // Draw up and down casts
    physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(carBodyPosition),
                                         Utils::glmToBullet(car->upCastPosition),
                                         btVector3(2.0f * (Car::kFarDistance - car->upDistance), 2.0f * (car->upDistance), 0));
    physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(carBodyPosition),
                                         Utils::glmToBullet(car->downCastPosition),
                                         btVector3(2.0f * (Car::kFarDistance - car->downDistance), 2.0f * (car->downDistance), 0));
}

void Renderer::DrawVroad(PhysicsEngine &physicsEngine) {
    if (track->tag == NFS_3 || track->tag == NFS_4) {
        float vRoadDisplayHeight = 0.2f;
        uint32_t nVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;
        for (uint32_t vroad_Idx = 0; vroad_Idx < nVroad; ++vroad_Idx) {
            // Render COL Vroad? Should I use TRK VROAD to work across HS too?
            if (vroad_Idx < nVroad - 1) {
                COLVROAD curVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx];
                COLVROAD nextVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroad[vroad_Idx + 1];

                INTPT refPt = curVroad.refPt;
                INTPT refPtNext = nextVroad.refPt;

                glm::quat rotationMatrix = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));

                // Transform NFS3/4 coords into ONFS 3d space
                glm::vec3 vroadPoint = rotationMatrix * glm::vec3((refPt.x / 65536.0f) / 10.f, ((refPt.y / 65536.0f) / 10.f), (refPt.z / 65536.0f) / 10.f);
                glm::vec3 vroadPointNext = rotationMatrix * glm::vec3((refPtNext.x / 65536.0f) / 10.f, ((refPtNext.y / 65536.0f) / 10.f), (refPtNext.z / 65536.0f) / 10.f);

                // Add a little vertical offset so it's not clipping through track geometry
                vroadPoint.y += vRoadDisplayHeight;
                vroadPointNext.y += vRoadDisplayHeight;
                physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext), btVector3(1, 0, 1));
                physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext), btVector3(1, 0, 1));

                glm::vec3 curVroadRightVec = rotationMatrix* glm::vec3(curVroad.right.x/128.f, curVroad.right.y/128.f, curVroad.right.z/128.f);

                if(Config::get().useFullVroad){
                    glm::vec3 leftWall = ((curVroad.leftWall/65536.0f) / 10.f) * curVroadRightVec;
                    glm::vec3 rightWall = ((curVroad.rightWall/65536.0f) / 10.f) * curVroadRightVec;

                    physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint - leftWall), btVector3(1, 0, 0.5f));
                    physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint + rightWall), btVector3(1, 0, 0.5f));
                } else {
                    physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint + curVroadRightVec), btVector3(1, 0, 0.5f));
                    physicsEngine.mydebugdrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint - curVroadRightVec), btVector3(1, 0, 0.5f));
                }
            }
        }
    }
}

void Renderer::DrawCameraAnimation(Camera &camera, PhysicsEngine &physicsEngine) {
    if (track->tag != NFS_3_PS1) {
        for (uint8_t can_Idx = 0; can_Idx < track->cameraAnimations.size(); ++can_Idx) {
            if (can_Idx < track->cameraAnimations.size() - 1) {
                // Draw CAN positions
                SHARED::CANPT refPt = track->cameraAnimations[can_Idx];
                SHARED::CANPT refPtNext = track->cameraAnimations[can_Idx + 1];
                glm::vec3 vroadPoint = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) *
                                       glm::vec3((refPt.x / 65536.0f) / 10.f, ((refPt.y / 65536.0f) / 10.f),
                                                 (refPt.z / 65536.0f) / 10.f);
                glm::vec3 vroadPointNext = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) *
                                           glm::vec3((refPtNext.x / 65536.0f) / 10.f,
                                                     ((refPtNext.y / 65536.0f) / 10.f),
                                                     (refPtNext.z / 65536.0f) / 10.f);
                vroadPoint.y += 0.2f;
                vroadPointNext.y += 0.2f;
                physicsEngine.mydebugdrawer.drawLine(
                        Utils::glmToBullet(vroadPoint + camera.initialPosition),
                        Utils::glmToBullet(vroadPointNext + camera.initialPosition), btVector3(0, 1, 1));

                // Draw Rotations
                glm::quat RotationMatrix =
                        glm::normalize(glm::quat(glm::vec3(glm::radians(0.f), glm::radians(-90.f), 0))) *
                        glm::normalize(
                                glm::quat(refPt.od1 / 65536.0f, refPt.od2 / 65536.0f, refPt.od3 / 65536.0f,
                                          refPt.od4 / 65536.0f));
                glm::vec3 direction = glm::normalize(vroadPoint * glm::inverse(RotationMatrix));
                physicsEngine.mydebugdrawer.drawLine(
                        Utils::glmToBullet(vroadPoint + camera.initialPosition),
                        Utils::glmToBullet(vroadPoint + camera.initialPosition + direction),
                        btVector3(0, 0.5, 0.5));
            }
        }
    }
}

void Renderer::UpdateShaders() {
    trackRenderer.trackShader.shaders.UpdatePrograms();
    trackRenderer.billboardShader.shaders.UpdatePrograms();
    carRenderer.carShader.shaders.UpdatePrograms();
    skyRenderer.skydomeShader.shaders.UpdatePrograms();
    shadowMapRenderer.depthShader.shaders.UpdatePrograms();
}

