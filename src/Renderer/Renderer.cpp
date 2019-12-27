#include "Renderer.h"

Renderer::Renderer(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger,
                   const std::vector<NfsAssetList> &installedNFS, const std::shared_ptr<ONFSTrack> &currentTrack,
                   std::shared_ptr<Car> &currentCar) : m_logger(onfsLogger), m_nfsAssetList(installedNFS),
                                                       m_pWindow(glWindow), m_track(currentTrack) {
    this->_InitialiseIMGUI();
    LOG(DEBUG) << "Renderer Initialised";
}

GLFWwindow *Renderer::InitOpenGL(int resolutionX, int resolutionY, const std::string &windowName) {
    // Initialise GLFW
    ASSERT(glfwInit(), "GLFW Init failed.\n");
    glfwSetErrorCallback(&Renderer::GlfwError);

    // TODO: Disable MSAA for now until texture array adds padding
    // glfwWindowHint(GLFW_SAMPLES, 2);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods

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



bool Renderer::Render(float totalTime, Camera &activeCamera, ParamData &userParams, AssetData &loadedAssets, std::shared_ptr<Car> &playerCar, const std::vector<CarAgent> &racers, PhysicsEngine &physicsEngine) {
    bool newAssetSelected = false;
    this->_NewFrame(userParams);

    // Perform frustum culling to get visible entities, from perspective of active camera
    std::vector<std::shared_ptr<Entity>> visibleEntities = _FrustumCull(m_track, activeCamera);

    // TODO: Move sun to an orbital manager class so the sunsets can look lit af
    GlobalLight sun;

    // Render the environment
    m_shadowMapRenderer.Render(userParams.nearPlane, userParams.farPlane, sun, m_track->textureArrayID, visibleEntities, playerCar, racers);
    m_skyRenderer.Render(activeCamera, sun, totalTime);
    m_trackRenderer.Render(playerCar, activeCamera, sun, m_track->textureArrayID, visibleEntities, userParams, m_shadowMapRenderer.m_depthTextureID, 0.5f);
    m_trackRenderer.RenderLights(activeCamera, m_track);

    // Render the Car and racers
    //std::vector<Light> carBodyContributingLights;
    //carRenderer.render(playerCar, camera, carBodyContributingLights);
    //for(auto &racer : racers){
    //    carRenderer.render(racer.car, camera, carBodyContributingLights);
    //}

    //if (ImGui::GetIO().MouseReleased[0] & userParams.windowActive) {
    //    targetedEntity = physicsEngine.CheckForPicking(camera.viewMatrix, camera.projectionMatrix, &entityTargeted);
    //}

    // DEBUG
    // this->_DrawFrustum(hermiteCamera, physicsEngine);
    // this->_DrawTrackCollision(m_track, physicsEngine);

    if (m_entityTargeted) {
        this->_DrawMetadata(m_pTargetedEntity);
    }

    if (this->_DrawMenuBar(loadedAssets)) {
        newAssetSelected = true;
    };

    this->_DrawUI(userParams, activeCamera, playerCar);
    glfwSwapBuffers(m_pWindow);

    return newAssetSelected;
}

std::vector<std::shared_ptr<Entity>> Renderer::_FrustumCull(const std::shared_ptr<ONFSTrack> track, Camera &camera)
{
    // Only update the frustum of the camera when actually culling, to save some performance
    camera.UpdateFrustum();

    // Perform frustum culling using the active camera frustum HI_DEBUG
    std::vector<std::shared_ptr<Entity>> visibleEntities;
    for(auto &trackblock : track->trackBlocks)
    {
        for(auto &trackEntity : trackblock.track)
        {
            if(camera.viewFrustum.CheckIntersection(trackEntity.GetAABB()))
            {
                visibleEntities.emplace_back(std::make_shared<Entity>(trackEntity));
            }
        }
        for(auto &objectEntity : trackblock.objects)
        {
            if(camera.viewFrustum.CheckIntersection(objectEntity.GetAABB()))
            {
                visibleEntities.emplace_back(std::make_shared<Entity>(objectEntity));
            }
        }
    }

    // TODO: Fix the AABB tree
    //auto aabbCollisions = m_track->cullTree.queryOverlaps(hermiteCamera.viewFrustum);
    //for(auto &collision : aabbCollisions)
    //{
    //    visibleEntities.emplace_back(std::static_pointer_cast<Entity>(collision));
    //}

    return visibleEntities;
}

void Renderer::_InitialiseIMGUI() {
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    const std::string glVersion = "#version " + ONFS_GL_VERSION;
    ImGui_ImplOpenGL3_Init(glVersion.c_str());
    ImGui::StyleColorsDark();
}

void Renderer::_DrawTrackCollision(const std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine)
{
    for(auto &trackblock : track->trackBlocks)
    {
        for(auto &trackEntity : trackblock.track)
        {
            _DrawAABB(trackEntity.GetAABB(), physicsEngine);
        }
        for(auto &objectEntity : trackblock.objects)
        {
            _DrawAABB(objectEntity.GetAABB(), physicsEngine);
        }
    }
}

void Renderer::_DrawAABB(const AABB &aabb, PhysicsEngine &physicsEngine)
{
    btVector3 colour = btVector3(0, 0, 0);
    physicsEngine.debugDrawer.drawBox(Utils::glmToBullet(aabb.position + aabb.min), Utils::glmToBullet(aabb.position + aabb.max), colour);
}

void Renderer::_DrawFrustum(Camera &camera, PhysicsEngine &physicsEngine)
{
    std::array<glm::vec3, 8> frustumDebugVizPoints = camera.viewFrustum.points;

    btVector3 colour(0, 1, 0);
    // FAR PLANE (TL to BR)
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[0]), Utils::glmToBullet(frustumDebugVizPoints[2]), colour);
    // NEAR ID (TR to BL)
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[5]), Utils::glmToBullet(frustumDebugVizPoints[7]), colour);

    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[0]), Utils::glmToBullet(frustumDebugVizPoints[1]), colour);
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[1]), Utils::glmToBullet(frustumDebugVizPoints[2]), colour);
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[2]), Utils::glmToBullet(frustumDebugVizPoints[3]), colour);
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[3]), Utils::glmToBullet(frustumDebugVizPoints[0]), colour);

    // NEAR PLANE
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[4]), Utils::glmToBullet(frustumDebugVizPoints[5]), colour);
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[5]), Utils::glmToBullet(frustumDebugVizPoints[6]), colour);
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[6]), Utils::glmToBullet(frustumDebugVizPoints[7]), colour);
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[7]), Utils::glmToBullet(frustumDebugVizPoints[4]), colour);

    // NEAR TO FAR
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[4]), Utils::glmToBullet(frustumDebugVizPoints[0]), colour);
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[5]), Utils::glmToBullet(frustumDebugVizPoints[1]), colour);
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[6]), Utils::glmToBullet(frustumDebugVizPoints[2]), colour);
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(frustumDebugVizPoints[7]), Utils::glmToBullet(frustumDebugVizPoints[3]), colour);
}

void Renderer::_DrawCarRaycasts(const std::shared_ptr<Car> &car, PhysicsEngine &physicsEngine)
{
    glm::vec3 carBodyPosition = car->carBodyModel.position;

    for(uint8_t rangeIdx = 0; rangeIdx < Car::kNumRangefinders; ++rangeIdx){
        physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(carBodyPosition),
                                             Utils::glmToBullet(car->castPositions[rangeIdx]),
                                             btVector3(2.0f * (Car::kFarDistance - car->rangefinders[rangeIdx]), 2.0f * (car->rangefinders[rangeIdx]), 0));
    }
    // Draw up and down casts
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(carBodyPosition),
                                         Utils::glmToBullet(car->upCastPosition),
                                         btVector3(2.0f * (Car::kFarDistance - car->upDistance), 2.0f * (car->upDistance), 0));
    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(carBodyPosition),
                                         Utils::glmToBullet(car->downCastPosition),
                                         btVector3(2.0f * (Car::kFarDistance - car->downDistance), 2.0f * (car->downDistance), 0));
}

void Renderer::_DrawVroad(const std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine)
{
    if (track->tag == NFS_3 || track->tag == NFS_4)
    {
        float vRoadDisplayHeight = 0.2f;
        uint32_t nVroad = boost::get<std::shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->col.vroadHead.nrec;
        for (uint32_t vroad_Idx = 0; vroad_Idx < nVroad; ++vroad_Idx)
        {
            // Render COL Vroad? Should I use TRK VROAD to work across HS too?
            if (vroad_Idx < nVroad - 1)
            {
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
                physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext), btVector3(1, 0, 1));
                physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPointNext), btVector3(1, 0, 1));

                glm::vec3 curVroadRightVec = rotationMatrix* glm::vec3(curVroad.right.x/128.f, curVroad.right.y/128.f, curVroad.right.z/128.f);

                if(Config::get().useFullVroad)
                {
                    glm::vec3 leftWall = ((curVroad.leftWall/65536.0f) / 10.f) * curVroadRightVec;
                    glm::vec3 rightWall = ((curVroad.rightWall/65536.0f) / 10.f) * curVroadRightVec;

                    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint - leftWall), btVector3(1, 0, 0.5f));
                    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint + rightWall), btVector3(1, 0, 0.5f));
                } else
                    {
                    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint + curVroadRightVec), btVector3(1, 0, 0.5f));
                    physicsEngine.debugDrawer.drawLine(Utils::glmToBullet(vroadPoint), Utils::glmToBullet(vroadPoint - curVroadRightVec), btVector3(1, 0, 0.5f));
                }
            }
        }
    }
}

void Renderer::_DrawCameraAnimation(Camera &camera, const std::shared_ptr<ONFSTrack> track, PhysicsEngine &physicsEngine)
{
    if (track->tag != NFS_3_PS1)
    {
        for (uint8_t can_Idx = 0; can_Idx < track->cameraAnimations.size(); ++can_Idx)
        {
            if (can_Idx < track->cameraAnimations.size() - 1)
            {
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
                physicsEngine.debugDrawer.drawLine(
                        Utils::glmToBullet(vroadPoint + camera.initialPosition),
                        Utils::glmToBullet(vroadPointNext + camera.initialPosition), btVector3(0, 1, 1));

                // Draw Rotations
                glm::quat RotationMatrix =
                        glm::normalize(glm::quat(glm::vec3(glm::radians(0.f), glm::radians(-90.f), 0))) *
                        glm::normalize(
                                glm::quat(refPt.od1 / 65536.0f, refPt.od2 / 65536.0f, refPt.od3 / 65536.0f,
                                          refPt.od4 / 65536.0f));
                glm::vec3 direction = glm::normalize(vroadPoint * glm::inverse(RotationMatrix));
                physicsEngine.debugDrawer.drawLine(
                        Utils::glmToBullet(vroadPoint + camera.initialPosition),
                        Utils::glmToBullet(vroadPoint + camera.initialPosition + direction),
                        btVector3(0, 0.5, 0.5));
            }
        }
    }
}

void Renderer::_DrawNFS34Metadata(Entity *targetEntity)
{
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

void Renderer::_DrawMetadata(Entity *targetEntity)
{
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
    switch (targetEntity->tag)
    {
        case NFSVer::NFS_3:
        case NFSVer::NFS_4:
            _DrawNFS34Metadata(targetEntity);
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

void Renderer::_DrawUI(ParamData &userParams, Camera &camera, std::shared_ptr<Car> &playerCar)
{
    // Draw Shadow Map
    ImGui::Begin("Shadow Map");
    ImGui::Image((ImTextureID) m_shadowMapRenderer.m_depthTextureID, ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, -1));
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
    ImGui::Text("X %f Y %f Z %f", camera.position.x, camera.position.y, camera.position.z);
    ImGui::Text("Block ID: %d", m_closestBlockID);
    ImGui::Text("Vroad ID: %d", CarAgent::getClosestVroad(playerCar, m_track));
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
        CarAgent::resetToVroad(0, 0, 0.f, m_track, playerCar);
    };
    ImGui::NewLine();
    ImGui::SameLine(0, 0.0f);
    if (!userParams.useNbData)
        ImGui::SliderInt("Draw Dist", &userParams.blockDrawDistance, 0, m_track->nBlocks / 2);
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
    glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Renderer::_DrawMenuBar(AssetData &loadedAssets)
{
    bool assetChange = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Track")) {
            for (auto &installedNFS : m_nfsAssetList) {
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
            for (auto &installedNFS : m_nfsAssetList) {
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

void Renderer::_NewFrame(ParamData &userParams)
{
    glClearColor(0.1f, 0.f, 0.5f, 1.f);
    glfwPollEvents();
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Detect a click on the 3D Window by detecting a click that isn't on ImGui
    userParams.windowActive = userParams.windowActive ? userParams.windowActive : (
            (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) && (!ImGui::GetIO().WantCaptureMouse));
    if (!userParams.windowActive) {
        ImGui::GetIO().MouseDrawCursor = false;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

