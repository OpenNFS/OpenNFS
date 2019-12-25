#include "Renderer.h"

Renderer::Renderer(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger,
                   const std::vector<NfsAssetList> &installedNFS, const std::shared_ptr<ONFSTrack> &currentTrack,
                   std::shared_ptr<Car> &currentCar) : logger(onfsLogger), installedNFSGames(installedNFS),
                                                   window(glWindow), track(currentTrack) {
    InitialiseIMGUI();
    LOG(DEBUG) << "Renderer Initialised";
}

GLFWwindow *Renderer::InitOpenGL(int resolutionX, int resolutionY, const std::string &windowName) {
    // Initialise GLFW
    ASSERT(glfwInit(), "GLFW Init failed.\n");
    glfwSetErrorCallback(&Renderer::GlfwError);

    // TODO: Disable MSAA for now until texture array adds padding
    // glfwWindowHint(GLFW_SAMPLES, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods

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

    GLint nTextureUnits, nMaxTextureLayers;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nTextureUnits);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &nMaxTextureLayers);

    LOG(DEBUG) << "Max Texture Units: " << nTextureUnits;
    LOG(DEBUG) << "Max Array Texture Layers: " << nMaxTextureLayers;
    LOG(DEBUG) << "OpenGL Initialisation successful";

    return window;
}

bool Renderer::Render(float totalTime, float deltaTime, Camera &camera, ParamData &userParams, AssetData &loadedAssets, std::shared_ptr<Car> &playerCar, std::vector<CarAgent> racers, PhysicsEngine &physicsEngine) {
    bool newAssetSelected = false;
    NewFrame(userParams);

    if (userParams.attachCamToHermite) {
        camera.UseSpline(totalTime);
    } else if (userParams.attachCamToCar) {
        // Compute MVP from keyboard and mouse, centered around a target car
        camera.FollowCar(playerCar, userParams.windowActive);
    } else {
        // Compute the MVP matrix from keyboard and mouse input
        camera.ComputeMatricesFromInputs(userParams.windowActive, deltaTime);
    }

    // Perform primitive frustum culling
    std::vector<std::shared_ptr<Entity>> visibleEntities;
    auto aabbCollisions = track->cullTree.queryOverlaps(camera.viewFrustum);
    for(auto &collision : aabbCollisions)
    {
        visibleEntities.emplace_back(std::static_pointer_cast<Entity>(collision));
    }

    // TODO: Move sun to an orbital manager class so the sunsets can look lit af
    GlobalLight sun;

    shadowMapRenderer.Render(userParams.nearPlane, userParams.farPlane, sun, track->textureArrayID, visibleEntities, playerCar, racers);
    skyRenderer.Render(camera, sun, totalTime);
    trackRenderer.Render(playerCar, camera, sun, track->textureArrayID, visibleEntities, userParams, shadowMapRenderer.m_depthTextureID, 0.5f);
    trackRenderer.RenderLights(camera, track);


    // Render the Car and racers
    //std::vector<Light> carBodyContributingLights;
    //carRenderer.render(playerCar, camera, carBodyContributingLights);
    //for(auto &racer : racers){
    //    carRenderer.render(racer.car, camera, carBodyContributingLights);
    //}

    //if (ImGui::GetIO().MouseReleased[0] & userParams.windowActive) {
    //    targetedEntity = physicsEngine.CheckForPicking(camera.viewMatrix, camera.projectionMatrix, &entityTargeted);
    //}

    if (entityTargeted) {
        DrawMetadata(targetedEntity);
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
    std::string glVersion = "#version " + ONFS_GL_VERSION;
    ImGui_ImplOpenGL3_Init(glVersion.c_str());
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
        case NFSVer::NFS_2_PS1:
        case NFSVer::NFS_2_SE:
            break;
        case NFSVer::NFS_3_PS1:
            break;
        case NFSVer::NFS_5:
            break;
    }
    ImGui::End();
}

void Renderer::DrawUI(ParamData &userParams, Camera &camera, std::shared_ptr<Car> &playerCar) {
    // Draw Shadow Map
    ImGui::Begin("Shadow Map");
    ImGui::Image((ImTextureID) shadowMapRenderer.m_depthTextureID, ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, -1));
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
    ImGui::Text("Block ID: %d", closestBlockID);
    ImGui::Text("Vroad ID: %d", CarAgent::getClosestVroad(playerCar, track));
    // ImGui::Text("Frustrum Objects: %d", physicsEngine.numObjects);
    ImGui::Checkbox("Frustum Cull", &userParams.frustumCull);
    ImGui::Checkbox("Raycast Viz", &userParams.drawRaycast);
    ImGui::Checkbox("AI Sim", &userParams.simulateCars);
    ImGui::Checkbox("Vroad Viz", &userParams.drawVroad);
    ImGui::Checkbox("CAN Debug", &userParams.drawCAN);

    if (ImGui::Button("Reset View")) {
        camera.ResetView();
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
        char meshDetailBuf[200];
        sprintf(meshDetailBuf, "%s (V: %zu)", playerCar->carBodyModel.m_name.c_str(), playerCar->carBodyModel.m_vertices.size());
        ImGui::Checkbox(meshDetailBuf, &playerCar->carBodyModel.enabled);
        ImGui::Checkbox(playerCar->leftFrontWheelModel.m_name.c_str(), &playerCar->leftFrontWheelModel.enabled);
        ImGui::Checkbox(playerCar->leftRearWheelModel.m_name.c_str(), &playerCar->leftRearWheelModel.enabled);
        ImGui::Checkbox(playerCar->rightFrontWheelModel.m_name.c_str(), &playerCar->rightFrontWheelModel.enabled);
        ImGui::Checkbox(playerCar->rightRearWheelModel.m_name.c_str(), &playerCar->rightRearWheelModel.enabled);
        ImGui::TreePop();
        if (ImGui::TreeNode("Misc Models")) {
            for (auto &mesh : playerCar->miscModels) {
                sprintf(meshDetailBuf, "%s (V: %zu)", mesh.m_name.c_str(), mesh.m_vertices.size());
                ImGui::Checkbox(meshDetailBuf, &mesh.enabled);
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
    glClearColor(0.1f, 0.f, 0.5f, 1.f);
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

