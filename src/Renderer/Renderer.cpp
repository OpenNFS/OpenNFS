#include "Renderer.h"

Renderer::Renderer(const std::shared_ptr<GLFWwindow> &window,
                   const std::shared_ptr<Logger> &onfsLogger,
                   const std::vector<NfsAssetList> &installedNFS,
                   const std::shared_ptr<Track> &currentTrack,
                   const std::shared_ptr<BulletDebugDrawer> &debugDrawer) :
    m_logger(onfsLogger), m_nfsAssetList(installedNFS), m_window(window), m_track(currentTrack), m_debugRenderer(debugDrawer) {
    this->_InitialiseIMGUI();
    LOG(DEBUG) << "Renderer Initialised";
}

std::shared_ptr<GLFWwindow> Renderer::InitOpenGL(uint32_t resolutionX, uint32_t resolutionY, const std::string &windowName) {
    // Initialise GLFW
    ASSERT(glfwInit(), "GLFW Init failed.\n");
    glfwSetErrorCallback(&Renderer::GlfwError);

    // TODO: Disable MSAA for now until texture array adds padding
    // glfwWindowHint(GLFW_SAMPLES, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods

    auto window = std::shared_ptr<GLFWwindow>(glfwCreateWindow(resolutionX, resolutionY, windowName.c_str(), nullptr, nullptr), [](GLFWwindow *w) { glfwTerminate(); });

    if (window == nullptr) {
        LOG(WARNING) << "Failed to create a GLFW window";
        getchar();
        glfwTerminate();
    }
    glfwMakeContextCurrent(window.get());
    glfwSetWindowSizeCallback(window.get(), Renderer::WindowSizeCallback);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        LOG(WARNING) << "Failed to initialize GLEW";
        getchar();
        glfwTerminate();
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window.get(), GLFW_STICKY_KEYS, GL_TRUE);
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

bool Renderer::Render(float totalTime,
                      const BaseCamera &activeCamera,
                      const HermiteCamera &hermiteCamera,
                      const std::shared_ptr<GlobalLight> &activeLight,
                      ParamData &userParams,
                      AssetData &loadedAssets,
                      const std::vector<std::shared_ptr<CarAgent>> &racers) {
    bool newAssetSelected = false;

    // Perform frustum culling to get visible entities, from perspective of active camera
    VisibleSet visibleSet = _FrustumCull(m_track, activeCamera, userParams);
    visibleSet.lights.insert(visibleSet.lights.begin(), activeLight);

    if (userParams.drawHermiteFrustum) {
        m_debugRenderer.DrawFrustum(hermiteCamera);
    }

    if (userParams.drawTrackAABB) {
        m_debugRenderer.DrawTrackCollision(m_track);
    }

    if (userParams.drawVroad) {
        m_debugRenderer.DrawVroad(m_track);
    }

    // Render the environment
    m_shadowMapRenderer.Render(userParams.nearPlane, userParams.farPlane, activeLight, m_track->textureArrayID, visibleSet.entities, racers);
    m_skyRenderer.Render(activeCamera, activeLight, totalTime);
    m_trackRenderer.Render(racers, activeCamera, m_track->textureArrayID, visibleSet.entities, visibleSet.lights, userParams, m_shadowMapRenderer.m_depthTextureID, 0.5f);
    m_trackRenderer.RenderLights(activeCamera, visibleSet.lights);
    m_debugRenderer.Render(activeCamera);

    // Render the Car and racers
    for (auto &racer : racers) {
        m_carRenderer.Render(racer->vehicle, activeCamera, visibleSet.lights);
    }

    if (this->_DrawMenuBar(loadedAssets)) {
        newAssetSelected = true;
    }

    // Render the Debug UI
    this->_DrawDebugUI(userParams, activeCamera);

    // Render the Game UI
    m_menuRenderer.Render();

    glfwSwapBuffers(m_window.get());

    return newAssetSelected;
}

VisibleSet Renderer::_FrustumCull(const std::shared_ptr<Track> &track, const BaseCamera &camera, ParamData &userParams) {
    VisibleSet visibleSet;

    // Perform frustum culling on the current camera, on local trackblocks
    for (auto &trackBlockID : _GetLocalTrackBlockIDs(track, camera, userParams)) {
        for (auto &trackEntity : track->trackBlocks[trackBlockID].track) {
            if (!userParams.useFrustumCull || camera.viewFrustum.CheckIntersection(trackEntity.GetAABB())) {
                visibleSet.entities.emplace_back(std::make_shared<Entity>(trackEntity));
            }
        }
        for (auto &objectEntity : track->trackBlocks[trackBlockID].objects) {
            if (!userParams.useFrustumCull || camera.viewFrustum.CheckIntersection(objectEntity.GetAABB())) {
                visibleSet.entities.emplace_back(std::make_shared<Entity>(objectEntity));
            }
        }
        for (auto &laneEntity : track->trackBlocks[trackBlockID].lanes) {
            // It's not worth checking for Lane AABB intersections
            visibleSet.entities.emplace_back(std::make_shared<Entity>(laneEntity));
        }
        for (auto &lightEntity : track->trackBlocks[trackBlockID].lights) {
            if (!userParams.useFrustumCull || camera.viewFrustum.CheckIntersection(lightEntity.GetAABB())) {
                visibleSet.lights.emplace_back(std::get<shared_ptr<BaseLight>>(lightEntity.raw));
            }
        }
    }

    // Global Objects are always visible
    for (auto &globalEntity : track->globalObjects) {
        visibleSet.entities.emplace_back(std::make_shared<Entity>(globalEntity));
    }

    // TODO: Fix the AABB tree
    // auto aabbCollisions = track->cullTree.queryOverlaps(camera->viewFrustum);
    // for(auto &collision : aabbCollisions)
    //{
    //    visibleEntities.emplace_back(std::static_pointer_cast<Entity>(collision));
    //}

    return visibleSet;
}

std::vector<uint32_t> Renderer::_GetLocalTrackBlockIDs(const std::shared_ptr<Track> &track, const BaseCamera &camera, ParamData &userParams) {
    std::vector<uint32_t> activeTrackBlockIds;
    uint32_t closestBlockID = 0;

    float lowestDistance = FLT_MAX;

    // Get closest track block to camera position
    for (auto &trackblock : track->trackBlocks) {
        float distance = glm::distance(camera.position, trackblock.position);
        if (distance < lowestDistance) {
            closestBlockID = trackblock.id;
            lowestDistance = distance;
        }
    }

    if (userParams.useNbData) {
        // Use the provided neighbour data to work out which blocks to render
        activeTrackBlockIds = track->trackBlocks[closestBlockID].neighbourIds;
    } else {
        // Use a draw distance value to return closestBlock +- drawDistance inclusive blocks
        for (auto trackblockIdx = closestBlockID - userParams.blockDrawDistance; trackblockIdx < closestBlockID + userParams.blockDrawDistance; ++trackblockIdx) {
            uint32_t activeBlock = trackblockIdx < 0 ? ((uint32_t) track->trackBlocks.size() + trackblockIdx) : (trackblockIdx % (uint32_t) track->trackBlocks.size());
            activeTrackBlockIds.emplace_back(activeBlock);
        }
    }

    return activeTrackBlockIds;
}

void Renderer::_InitialiseIMGUI() {
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
    const std::string glVersion = "#version " + ONFS_GL_VERSION;
    ImGui_ImplOpenGL3_Init(glVersion.c_str());
    ImGui::StyleColorsDark();
}

void Renderer::DrawMetadata(Entity *targetEntity) {
    ImGui::Begin("Engine Entity");
    ImGui::Text("%s", get_string(targetEntity->tag).c_str());
    ImGui::Text("%s", get_string(targetEntity->type).c_str());
    // Only display these if they're relevant
    if (targetEntity->parentTrackblockID != -1) {
        ImGui::Text("TrkBlk: %d", targetEntity->parentTrackblockID);
    }
    if (targetEntity->entityID != -1) {
        ImGui::Text("ID: %d", targetEntity->entityID);
    }
    ImGui::Separator();

    // Traverse the loader structures and print pretty with IMGUI
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
        std::shared_ptr<BaseLight> targetBaseLight = std::get<std::shared_ptr<BaseLight>>(targetEntity->raw);
        std::shared_ptr<TrackLight> targetLight    = std::static_pointer_cast<TrackLight>(targetBaseLight);
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
        ImGui::Text("Type: %hhu", targetLight->type);
        ImGui::Text("Unknowns: ");
        ImGui::Text("[1]: %d", targetLight->unknown1);
        ImGui::Text("[2]: %d", targetLight->unknown2);
        ImGui::Text("[3]: %d", targetLight->unknown3);
        ImGui::Text("[4]: %f", targetLight->unknown4);
    } break;
    case EntityType::ROAD:
        break;
    case EntityType::XOBJ:
        break;
    case EntityType::SOUND:
        break;
    case EntityType::VROAD_CEIL:
        break;
    case EntityType::CAR:
        Car *targetCar = std::get<Car *>(targetEntity->raw);
        ImGui::Text("%s Supported Colours:", targetCar->name.c_str());
        for (auto &carColour : targetCar->assetData.colours) {
            ImVec4 carColourIm(carColour.colour.x, carColour.colour.y, carColour.colour.z, 0);
            ImGui::ColorEdit4(carColour.colourName.c_str(), (float *) &carColourIm); // Edit 3 floats representing a color
        }
        ImGui::Text("Ray Distances U: %f F: %f R: %f L: %f",
                    targetCar->rangefinderInfo.upDistance,
                    targetCar->rangefinderInfo.rangefinders[RayDirection::FORWARD_RAY],
                    targetCar->rangefinderInfo.rangefinders[RayDirection::RIGHT_RAY],
                    targetCar->rangefinderInfo.rangefinders[RayDirection::LEFT_RAY]);
        ImGui::Text("Speed %f", targetCar->GetVehicle()->getCurrentSpeedKmHour() / 10.f);
        // Physics Parameters
        ImGui::SliderFloat("Engine Force", &targetCar->vehicleState.gEngineForce, 0, 10000.0f);
        ImGui::SliderFloat("Breaking Force", &targetCar->vehicleState.gBreakingForce, 0, 1000.0f);
        ImGui::SliderFloat("Max Engine Force", &targetCar->vehicleProperties.maxEngineForce, 0, 10000.0f);
        ImGui::SliderFloat("Max Breaking Force", &targetCar->vehicleProperties.maxBreakingForce, 0, 1000.0f);
        ImGui::SliderFloat("Susp Rest.", &targetCar->vehicleProperties.suspensionRestLength, 0, 0.1f); // btScalar(0.030);
        ImGui::SliderFloat("Susp Stiff.", &targetCar->vehicleProperties.suspensionStiffness, 0, 1000.f);
        ImGui::SliderFloat("Susp Damp.", &targetCar->vehicleProperties.suspensionDamping, 0, 1000.f);
        ImGui::SliderFloat("Susp Compr.", &targetCar->vehicleProperties.suspensionCompression, 0, 1000.f);
        ImGui::SliderFloat("Friction.", &targetCar->vehicleProperties.wheelFriction, 0, 1.f);
        ImGui::SliderFloat("Roll Infl.", &targetCar->vehicleProperties.rollInfluence, 0, 0.5);
        ImGui::SliderFloat("Steer Incr.", &targetCar->vehicleProperties.steeringIncrement, 0.f, 0.1f);
        ImGui::SliderFloat("Steer Clamp", &targetCar->vehicleProperties.steeringClamp, 0.f, 0.5f);
        ImGui::Text("Roll (deg) x: %f y: %f z: %f",
                    glm::eulerAngles(targetCar->carBodyModel.orientation).x * 180 / SIMD_PI,
                    glm::eulerAngles(targetCar->carBodyModel.orientation).y * 180 / SIMD_PI,
                    glm::eulerAngles(targetCar->carBodyModel.orientation).z * 180 / SIMD_PI);
        break;
    }
    ImGui::Text("Object Flags: %d", targetEntity->flags);
    ImGui::Text("Collideable: %s", targetEntity->collideable ? "Yes" : "No");
    ImGui::Text("Dynamic: %s", targetEntity->dynamic ? "Yes" : "No");
    ImGui::End();
}

void Renderer::_DrawDebugUI(ParamData &userParams, const BaseCamera &camera) {
    // Draw Shadow Map
    ImGui::Begin("Shadow Map");
    ImGui::Image(reinterpret_cast<ImTextureID>(m_shadowMapRenderer.m_depthTextureID), ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, -1));
    ImGui::SliderFloat("Near Plane", &userParams.nearPlane, 0, 300);
    ImGui::SliderFloat("Far Plane", &userParams.farPlane, 0, 300);
    ImGui::End();
    // Draw Logger UI
    m_logger->onScreenLog.Draw("ONFS Log");
    // Draw UI (Tactically)
    ImGui::Text("OpenNFS Engine");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::SliderFloat("Time Scale Factor", &userParams.timeScaleFactor, 0, 10);
    ImGui::Checkbox("Frustum Cull", &userParams.useFrustumCull);
    ImGui::Checkbox("Bullet Debug View", &userParams.physicsDebugView);
    ImGui::Checkbox("Classic Graphics", &userParams.useClassicGraphics);
    ImGui::Checkbox("Hermite Curve Cam", &userParams.attachCamToHermite);
    ImGui::Checkbox("Car Cam", &userParams.attachCamToCar);
    ImGui::Text("X %f Y %f Z %f", camera.position.x, camera.position.y, camera.position.z);
    ImGui::Checkbox("Draw Herm Frustum", &userParams.drawHermiteFrustum);
    ImGui::Checkbox("Draw Track AABBs", &userParams.drawTrackAABB);
    ImGui::Checkbox("Raycast Viz", &userParams.drawRaycast);
    ImGui::Checkbox("AI Sim", &userParams.simulateCars);
    ImGui::Checkbox("Vroad Viz", &userParams.drawVroad);
    ImGui::Checkbox("CAN Debug", &userParams.drawCAN);
    ImGui::SameLine(0, -1.0f);
    ImGui::NewLine();
    ImGui::SameLine(0, 0.0f);
    if (!userParams.useNbData) {
        ImGui::SliderInt("Draw Dist", &userParams.blockDrawDistance, 0, m_track->nBlocks / 2);
    }
    ImGui::Checkbox("NBData", &userParams.useNbData);
    ImGui::NewLine();
    ImGui::ColorEdit3("Sun Atten", (float *) &userParams.sunAttenuation); // Edit 3 floats representing a color

    ImGui::SliderFloat("Track Specular Damper", &userParams.trackSpecDamper, 0, 100);
    ImGui::SliderFloat("Track Specular Reflectivity", &userParams.trackSpecReflectivity, 0, 10);

    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(m_window.get(), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Renderer::_DrawMenuBar(AssetData &loadedAssets) {
    bool assetChange = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Track")) {
            for (auto &installedNFS : m_nfsAssetList) {
                if (ImGui::BeginMenu(get_string(installedNFS.tag).c_str())) {
                    for (auto &track : installedNFS.tracks) {
                        if (ImGui::MenuItem(track.c_str())) {
                            loadedAssets.trackTag = installedNFS.tag;
                            loadedAssets.track    = track;
                            assetChange           = true;
                        }
                    }
                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Car")) {
            for (auto &installedNFS : m_nfsAssetList) {
                if (ImGui::BeginMenu(get_string(installedNFS.tag).c_str())) {
                    for (auto &car : installedNFS.cars) {
                        if (ImGui::MenuItem(car.c_str())) {
                            loadedAssets.carTag = installedNFS.tag;
                            loadedAssets.car    = car;
                            assetChange         = true;
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
    glfwSetMouseButtonCallback(m_window.get(), nullptr);
    glfwSetScrollCallback(m_window.get(), nullptr);
    glfwSetKeyCallback(m_window.get(), nullptr);
    glfwSetCharCallback(m_window.get(), nullptr);
}