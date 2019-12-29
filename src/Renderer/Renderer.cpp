#include "Renderer.h"

Renderer::Renderer(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger,
                   const std::vector<NfsAssetList> &installedNFS, std::shared_ptr<ONFSTrack> currentTrack, std::shared_ptr<BulletDebugDrawer> debugDrawer) :
        m_logger(onfsLogger), m_nfsAssetList(installedNFS), m_pWindow(glWindow), m_track(currentTrack), m_debugRenderer(debugDrawer)
{
    this->_InitialiseIMGUI();
    LOG(DEBUG) << "Renderer Initialised";
}

GLFWwindow *Renderer::InitOpenGL(int resolutionX, int resolutionY, const std::string &windowName)
{
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

    if (window == nullptr)
    {
        LOG(WARNING) << "Failed to create a GLFW window.";
        getchar();
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, Renderer::WindowSizeCallback);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
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

bool Renderer::Render(float totalTime, const std::shared_ptr<Camera> &activeCamera, const std::shared_ptr<HermiteCamera> &hermiteCamera, ParamData &userParams, AssetData &loadedAssets, const std::vector<CarAgent> &racers)
{
    bool newAssetSelected = false;

    // Perform frustum culling to get visible entities, from perspective of active camera
    std::vector<std::shared_ptr<Entity>> visibleEntities = _FrustumCull(m_track, hermiteCamera, userParams);

    if (userParams.drawHermiteFrustum)
    {
        m_debugRenderer.DrawFrustum(hermiteCamera);
    }

    if (userParams.drawTrackAABB)
    {
        m_debugRenderer.DrawTrackCollision(m_track);
    }

    // Render the environment
    m_shadowMapRenderer.Render(userParams.nearPlane, userParams.farPlane, m_sun, m_track->textureArrayID, visibleEntities, racers);
    m_skyRenderer.Render(activeCamera, m_sun, totalTime);
    m_trackRenderer.Render(racers, activeCamera, m_sun, m_track->textureArrayID, visibleEntities, userParams, m_shadowMapRenderer.m_depthTextureID, 0.5f);
    m_trackRenderer.RenderLights(activeCamera, m_track);
    m_debugRenderer.Render(activeCamera);

    // Render the Car and racers
    std::vector<Light> carBodyContributingLights;
    for(auto &racer : racers){
        m_carRenderer.Render(racer.vehicle, activeCamera, carBodyContributingLights);
    }

    //if (ImGui::GetIO().MouseReleased[0] & userParams.windowActive) {
    //    targetedEntity = physicsEngine.CheckForPicking(camera->viewMatrix, camera->projectionMatrix, &entityTargeted);
    //}

    if (m_entityTargeted)
    {
        this->_DrawMetadata(m_pTargetedEntity);
    }

    if (this->_DrawMenuBar(loadedAssets))
    {
        newAssetSelected = true;
    }

    this->_DrawUI(userParams, activeCamera);

    glfwSwapBuffers(m_pWindow);

    return newAssetSelected;
}

std::vector<std::shared_ptr<Entity>> Renderer::_FrustumCull(const std::shared_ptr<ONFSTrack> &track, const std::shared_ptr<Camera> &camera, ParamData &userParams)
{
    // Only update the frustum of the camera when actually culling, to save some performance
    camera->UpdateFrustum();

    // Perform frustum culling on the current camera, on local trackblocks
    std::vector<std::shared_ptr<Entity>> visibleEntities;
    for (auto &trackBlockID : _GetLocalTrackBlockIDs(track, camera, userParams))
    {
        for (auto &trackEntity : track->trackBlocks[trackBlockID].track)
        {
            if (camera->viewFrustum.CheckIntersection(trackEntity.GetAABB()))
            {
                visibleEntities.emplace_back(std::make_shared<Entity>(trackEntity));
            }
        }
        for (auto &objectEntity : track->trackBlocks[trackBlockID].objects)
        {
            if (camera->viewFrustum.CheckIntersection(objectEntity.GetAABB()))
            {
                visibleEntities.emplace_back(std::make_shared<Entity>(objectEntity));
            }
        }
        for (auto &laneEntity : track->trackBlocks[trackBlockID].lanes)
        {
            // It's not worth checking for Lane AABB intersections
            //if(camera->viewFrustum.CheckIntersection(objectEntity.GetAABB()))
            {
                visibleEntities.emplace_back(std::make_shared<Entity>(laneEntity));
            }
        }
    }

    // TODO: Fix the AABB tree
    //auto aabbCollisions = track->cullTree.queryOverlaps(camera->viewFrustum);
    //for(auto &collision : aabbCollisions)
    //{
    //    visibleEntities.emplace_back(std::static_pointer_cast<Entity>(collision));
    //}

    return visibleEntities;
}

std::vector<uint32_t> Renderer::_GetLocalTrackBlockIDs(const std::shared_ptr<ONFSTrack> &track, const std::shared_ptr<Camera> &camera, ParamData &userParams)
{
    std::vector<uint32_t> activeTrackBlockIds;
    uint32_t closestBlockID = 0;

    float lowestDistance = FLT_MAX;

    // Get closest track block to camera position
    for (auto &trackblock :  track->trackBlocks)
    {
        float distance = glm::distance(camera->position, trackblock.center);
        if (distance < lowestDistance)
        {
            closestBlockID = trackblock.blockId;
            lowestDistance = distance;
        }
    }

    // If we have an NFS3 track loaded, use the provided neighbour data to work out which blocks to render
    if ((track->tag == NFS_3 || track->tag == NFS_4) && userParams.useNbData)
    {
        for (auto &neighbourBlockData : boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[closestBlockID].nbdData)
        {
            if (neighbourBlockData.blk == -1)
            {
                break;
            }
            else
            {
                activeTrackBlockIds.emplace_back(neighbourBlockData.blk);
            }
        }
    }
    else
    {
        // Use a draw distance value to return closestBlock +- drawDistance inclusive blocks
        for (auto trackblockIdx = closestBlockID - userParams.blockDrawDistance; trackblockIdx < closestBlockID + userParams.blockDrawDistance; ++trackblockIdx)
        {
            uint32_t activeBlock = trackblockIdx < 0 ? ((uint32_t) track->trackBlocks.size() + trackblockIdx) : (trackblockIdx % (uint32_t) track->trackBlocks.size());
            activeTrackBlockIds.emplace_back(activeBlock);
        }
    }

    return activeTrackBlockIds;
}

void Renderer::_InitialiseIMGUI()
{
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    const std::string glVersion = "#version " + ONFS_GL_VERSION;
    ImGui_ImplOpenGL3_Init(glVersion.c_str());
    ImGui::StyleColorsDark();
}

void Renderer::_DrawMetadata(Entity *targetEntity)
{
    ImGui::Begin("Engine Entity");
    ImGui::Text("%s", ToString(targetEntity->tag));
    ImGui::Text("%s", ToString(targetEntity->type));
    // Only display these if they're relevant
    if (targetEntity->parentTrackblockID != -1)
    {
        ImGui::Text("TrkBlk: %d", targetEntity->parentTrackblockID);
    }
    if (targetEntity->entityID != -1)
    {
        ImGui::Text("ID: %d", targetEntity->entityID);
    }
    ImGui::Separator();

    // Traverse the loader structures and print pretty with IMGUI
    switch (targetEntity->type)
    {
        case EntityType::VROAD:
            break;
        case EntityType::OBJ_POLY:
            break;
        case EntityType::GLOBAL:
            break;
        case EntityType::LANE:
            break;
        case EntityType::LIGHT:
        {
            Light *targetLight = &boost::get<Light>(targetEntity->glMesh);
            ImVec4 lightColour(targetLight->colour.x, targetLight->colour.y, targetLight->colour.z, targetLight->colour.w);
            ImVec4 lightAttenuation(targetLight->attenuation.x, targetLight->attenuation.y, targetLight->attenuation.z,0.0f);
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
        case EntityType::VROAD_CEIL:
            break;
        case EntityType::CAR:
            Car *targetCar = boost::get<Car *>(targetEntity->glMesh);
            ImGui::Text("%s Supported Colours:", targetCar->name.c_str());
            for (auto &carColour : targetCar->assetData.colours)
            {
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

void Renderer::_DrawUI(ParamData &userParams, const std::shared_ptr<Camera> &camera)
{
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
    ImGui::Checkbox("Bullet Debug View", &userParams.physicsDebugView);
    ImGui::Checkbox("Classic Graphics", &userParams.useClassicGraphics);
    ImGui::Checkbox("Hermite Curve Cam", &userParams.attachCamToHermite);
    ImGui::Checkbox("Car Cam", &userParams.attachCamToCar);
    std::stringstream world_position_string;
    ImGui::Text("X %f Y %f Z %f", camera->position.x, camera->position.y, camera->position.z);
    // ImGui::Text("Block ID: %d", m_closestBlockID);
    // ImGui::Text("Vroad ID: %d", CarAgent::getClosestVroad(playerCar, m_track));
    ImGui::Checkbox("Frustum Cull", &userParams.frustumCull);
    ImGui::Checkbox("Draw Herm Frustum", &userParams.drawHermiteFrustum);
    ImGui::Checkbox("Draw Track AABBs", &userParams.drawTrackAABB);
    ImGui::Checkbox("Raycast Viz", &userParams.drawRaycast);
    ImGui::Checkbox("AI Sim", &userParams.simulateCars);
    ImGui::Checkbox("Vroad Viz", &userParams.drawVroad);
    ImGui::Checkbox("CAN Debug", &userParams.drawCAN);

    if (ImGui::Button("Reset View"))
    {
        camera->ResetView();
    };
    ImGui::SameLine(0, -1.0f);
    if (ImGui::Button("Reset Car to Start"))
    {
        // CarAgent::resetToVroad(0, 0, 0.f, m_track, playerCar);
    };
    ImGui::NewLine();
    ImGui::SameLine(0, 0.0f);
    if (!userParams.useNbData)
    {
        ImGui::SliderInt("Draw Dist", &userParams.blockDrawDistance, 0, m_track->nBlocks / 2);
    }
    ImGui::Checkbox("NBData", &userParams.useNbData);
    ImGui::NewLine();
    ImGui::ColorEdit3("Sun Atten", (float *) &userParams.sunAttenuation); // Edit 3 floats representing a color
    //ImGui::SliderFloat3("NFS2 Rot Dbg", (float *) &userParams.nfs2_rotate, -M_PI, M_PI);

    ImGui::SliderFloat("Track Specular Damper", &userParams.trackSpecDamper, 0, 100);
    ImGui::SliderFloat("Track Specular Reflectivity", &userParams.trackSpecReflectivity, 0, 10);

    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Renderer::_DrawMenuBar(AssetData &loadedAssets)
{
    bool assetChange = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Track"))
        {
            for (auto &installedNFS : m_nfsAssetList)
            {
                if (ImGui::BeginMenu(ToString(installedNFS.tag)))
                {
                    for (auto &track : installedNFS.tracks)
                    {
                        if (ImGui::MenuItem(track.c_str()))
                        {
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
        if (ImGui::BeginMenu("Car"))
        {
            for (auto &installedNFS : m_nfsAssetList)
            {
                if (ImGui::BeginMenu(ToString(installedNFS.tag)))
                {
                    for (auto &car : installedNFS.cars)
                    {
                        if (ImGui::MenuItem(car.c_str()))
                        {
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

Renderer::~Renderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // Fixes bug on next render creation when install_callbacks set to true for ImGui
    glfwSetMouseButtonCallback(m_pWindow, nullptr);
    glfwSetScrollCallback(m_pWindow, nullptr);
    glfwSetKeyCallback(m_pWindow, nullptr);
    glfwSetCharCallback(m_pWindow, nullptr);
}