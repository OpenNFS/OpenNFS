#include "Renderer.h"

#include "../Race/Agents/RacerAgent.h"
#include "../Race/OrbitalManager.h"
#include <backends/imgui_impl_opengl3.h>
#include <implot.h>

namespace OpenNFS {
    Renderer::Renderer(std::shared_ptr<GLFWwindow> const &window, std::shared_ptr<Logger> const &onfsLogger,
                       std::vector<NfsAssetList> installedNFS, std::shared_ptr<Track> const &currentTrack,
                       std::shared_ptr<BulletDebugDrawer> const &debugDrawer)
        : m_window(window), m_logger(onfsLogger), m_nfsAssetList(std::move(installedNFS)), m_track(currentTrack),
          m_debugRenderer(debugDrawer) {
        LOG(DEBUG) << "Renderer Initialised";
    }

    std::shared_ptr<GLFWwindow> Renderer::InitOpenGL(std::string const &windowName) {
        // Initialise GLFW
        CHECK_F(glfwInit() == GLFW_TRUE, "GLFW Init failed.\n");
        glfwSetErrorCallback(&Renderer::GlfwError);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods

        if (Config::get().resX == RESOLUTION_NOT_SET || Config::get().resY == RESOLUTION_NOT_SET) {
            // Get current screen resolution and use that
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            int monitorWidth, monitorHeight;
            glfwGetMonitorWorkarea(monitor, NULL, NULL, &monitorWidth, &monitorHeight);

            Config::get().resX = static_cast<uint32_t>(monitorWidth);
            Config::get().resY = static_cast<uint32_t>(monitorHeight);
        }
        auto window =
            std::shared_ptr<GLFWwindow>(glfwCreateWindow(Config::get().resX, Config::get().resY, windowName.c_str(), nullptr, nullptr),
                                        [](GLFWwindow *) { glfwTerminate(); });
        if (window == nullptr) {
            LOG(WARNING) << "Failed to create a GLFW window";
            glfwTerminate();
        }
        glfwMakeContextCurrent(window.get());
        glfwSetWindowSizeCallback(window.get(), WindowSizeCallback);

        // Initialize GLEW
        glewExperimental = GL_TRUE; // Needed for core profile
        if (glewInit() != GLEW_OK) {
            LOG(WARNING) << "Failed to initialize GLEW";
            getchar();
            glfwTerminate();
        }

        // Ensure we can capture the escape key being pressed below
        glfwSetInputMode(window.get(), GLFW_STICKY_KEYS, GL_TRUE);
        // Dark blue background
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

        glfwGetWindowSize(window.get(), reinterpret_cast<int *>(&Config::get().windowSizeX),
                          reinterpret_cast<int *>(&Config::get().windowSizeY));
        LOG(DEBUG) << "Window Size: " << Config::get().windowSizeX << "x" << Config::get().windowSizeY;

        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
        std::string const glVersion = "#version " + ONFS_GL_VERSION;
        ImGui_ImplOpenGL3_Init(glVersion.c_str());
        ImGui::StyleColorsDark();

        LOG(DEBUG) << "IMGUI Initialisation successful";

        return window;
    }

    bool Renderer::Render(float const totalTime, float const deltaTime, BaseCamera const &activeCamera, HermiteCamera const &hermiteCamera,
                          GlobalLight const *activeLight, ParamData &userParams, AssetData &loadedAssets,
                          std::vector<std::shared_ptr<CarAgent>> const &racers, std::optional<Entity *> const targetedEntity) {
        bool newAssetSelected = false;
        m_numRacers = racers.size();

        // Perform frustum culling to get visible entities, from perspective of active camera
        auto const [visibleEntities, visibleLights] = _FrustumCull(m_track, activeCamera, activeLight, racers[0], userParams);

        if (userParams.drawHermiteFrustum) {
            m_debugRenderer.DrawFrustum(hermiteCamera);
        }

        if (userParams.drawTrackAABB) {
            m_debugRenderer.DrawTrackCollision(m_track);
        }

        if (userParams.drawVroad) {
            m_debugRenderer.DrawVroad(m_track);
        }

        if (userParams.drawRaycast) {
            for (auto const &racer : racers) {
                m_debugRenderer.DrawVehicleRaycasts(racer->vehicle);
            }
        }

        if (userParams.drawNFS4PhysicsVectors) {
            for (auto const &racer : racers) {
                m_debugRenderer.DrawNFS4PhysicsDebug(racer->vehicle);
            }
        }

        if (userParams.drawCAN) {
            m_debugRenderer.DrawCameraAnimation(m_track);
        }

        // Render the environment (CSM shadow pass)
        m_shadowMapRenderer.Render(activeLight, activeCamera, m_track->textureArrayID, visibleEntities, racers);
        if (userParams.drawSkydome) {
            m_skyRenderer.Render(activeCamera, activeLight, totalTime, userParams.weatherMixFactor);
        }

        // Calculate ambient factor based on sun height - when sun is below horizon (y < 0), it's night
        float const ambientFactor = std::max(0.f, activeLight->position.y / (OrbitalManager::SKYDOME_RADIUS) * 0.5f);
        m_trackRenderer.Render(racers, activeCamera, m_track->textureArrayID, visibleEntities, visibleLights, userParams,
                               m_shadowMapRenderer.GetTextureArrayID(), ambientFactor);
        m_trackRenderer.RenderLights(activeCamera, visibleLights);
        m_debugRenderer.Render(activeCamera);
        if (userParams.drawMinimap) {
            m_miniMapRenderer.Render(m_track, racers);
        }

        // Render the Car and racers with CSM shadows
        for (auto &racer : racers) {
            m_carRenderer.Render(racer->vehicle, activeCamera, visibleLights, m_shadowMapRenderer.GetTextureArrayID());
        }

        if (userParams.drawAIState) {
            for (auto const &racer : racers) {
                if (racer->Type() == AI) {
                    m_debugRenderer.DrawVehicleAI(std::dynamic_pointer_cast<RacerAgent>(racer), activeCamera);
                }
            }
        }

        if (this->_DrawMenuBar(loadedAssets)) {
            newAssetSelected = true;
        }

        if (targetedEntity.has_value()) {
            _DrawMetadata(targetedEntity.value());
        }

        // Render the Debug UI
        this->_DrawDebugUI(userParams, deltaTime, activeCamera);

        if (userParams.showNFS4PhysicsDebug) {
            this->_DrawNFS4PhysicsDebug(racers);
        }

        return newAssetSelected;
    }

    uint32_t Renderer::GetCameraTargetVehicleID() const {
        return m_cameraTargetVehicleID;
    }

    void Renderer::NewFrame() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Renderer::EndFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    VisibleSet Renderer::_FrustumCull(std::shared_ptr<Track> const &track, BaseCamera const &camera, GlobalLight const *globalLight,
                                      std::shared_ptr<CarAgent> const &racer, ParamData const &userParams) {
        VisibleSet visibleSet;
        // The sun/moon should always contribute
        visibleSet.lights.push_back(globalLight);
        // As should the players car
        visibleSet.lights.push_back(&racer->vehicle->leftTailLight);
        visibleSet.lights.push_back(&racer->vehicle->rightTailLight);

        if (userParams.useFrustumCull) {
            // Perform frustum culling on the current camera, on local trackblocks
            for (auto &collision : track->cullTree.queryOverlaps(camera.viewFrustum)) {
                auto entity{std::dynamic_pointer_cast<Entity>(collision)};
                if (entity->Type() == EntityType::LIGHT) {
                    visibleSet.lights.emplace_back(entity->AsLight());
                } else {
                    visibleSet.entities.emplace_back(entity);
                }
            }
        } else {
            std::vector<uint32_t> const localTrackBlockIDs{_GetLocalTrackBlockIDs(track, camera)};
            for (auto const &localTrackBlockID : localTrackBlockIDs) {
                visibleSet.entities.insert(visibleSet.entities.end(), track->perTrackblockEntities.at(localTrackBlockID).begin(),
                                           track->perTrackblockEntities.at(localTrackBlockID).end());
            }
        }

        return visibleSet;
    }

    std::vector<uint32_t> Renderer::_GetLocalTrackBlockIDs(std::shared_ptr<Track> const &track, BaseCamera const &camera) {
        std::vector<uint32_t> activeTrackBlockIds;
        uint32_t nearestBlockID = 0;

        float lowestDistance = FLT_MAX;

        // Get the closest track block to camera position
        for (auto &trackblock : track->trackBlocks) {
            if (float const distance = glm::distance(camera.position, trackblock.position); distance < lowestDistance) {
                nearestBlockID = trackblock.id;
                lowestDistance = distance;
            }
        }

        TrackBlock const &nearestTrackBlock{track->trackBlocks[nearestBlockID]};
        // Use the provided neighbour data to work out which blocks to render if there is any
        if (!nearestTrackBlock.neighbourIds.empty()) {
            activeTrackBlockIds = nearestTrackBlock.neighbourIds;
        } else {
            constexpr int32_t kBlockDistance{15};
            // Use a draw distance value to return closestBlock +- kBlockDrawDistance inclusive blocks
            for (int32_t trackblockIdx{static_cast<int32_t>(nearestBlockID) - kBlockDistance};
                 trackblockIdx < nearestBlockID + kBlockDistance; ++trackblockIdx) {
                uint32_t activeBlock = trackblockIdx < 0 ? (static_cast<uint32_t>(track->trackBlocks.size()) + trackblockIdx)
                                                         : (trackblockIdx % static_cast<uint32_t>(track->trackBlocks.size()));
                activeTrackBlockIds.emplace_back(activeBlock);
            }
        }

        return activeTrackBlockIds;
    }

    void Renderer::_DrawMetadata(Entity const *targetEntity) {
        ImGui::Begin("Engine Entity");
        ImGui::Text("%s", magic_enum::enum_name(targetEntity->Type()).data());
        ImGui::Text("ID: %d", targetEntity->ID());
        ImGui::Separator();

        // Traverse the loader structures and print pretty with IMGUI
        switch (targetEntity->Type()) {
        case EntityType::LIGHT: {
            auto const targetBaseLight = targetEntity->AsLight();
            ImVec4 lightColour(targetBaseLight->colour.x, targetBaseLight->colour.y, targetBaseLight->colour.z, targetBaseLight->colour.w);
            ImVec4 lightAttenuation(targetBaseLight->attenuation.x, targetBaseLight->attenuation.y, targetBaseLight->attenuation.z, 0.0f);
            // Colour, type, attenuation, position and NFS unknowns
            ImGui::ColorEdit4("Light Colour", reinterpret_cast<float *>(&lightColour)); // Edit 3 floats representing a colour
            targetBaseLight->colour = glm::vec4(lightColour.x, lightColour.y, lightColour.z, lightColour.w);
            ImGui::SliderFloat3("Attenuation (A, B, C)", reinterpret_cast<float *>(&lightAttenuation), 0, 10.0f);
            targetBaseLight->attenuation = glm::vec3(lightAttenuation.x, lightAttenuation.y, lightAttenuation.z);
            ImGui::Text("x: %f y: %f z: %f ", targetBaseLight->position.x, targetBaseLight->position.y, targetBaseLight->position.z);
            ImGui::Separator();
            ImGui::Text("NFS Data");
            ImGui::Text("Type: %hhu", targetBaseLight->type);
        } break;
        default:
            break;
        }
        ImGui::Text("Object Flags: %d", targetEntity->RawFlags());
        ImGui::Text("Collidable: %s", targetEntity->Collidable() ? "Yes" : "No");
        ImGui::Text("Dynamic: %s", targetEntity->Dynamic() ? "Yes" : "No");
        ImGui::End();
    }

    void Renderer::_DrawDebugUI(ParamData &userParams, float const deltaTime, BaseCamera const &camera) {
        // Update deltatime history for smoothed FPS display
        m_deltaTimeHistory[m_deltaTimeHistoryIndex] = deltaTime;
        m_deltaTimeHistoryIndex = (m_deltaTimeHistoryIndex + 1) % kDeltaTimeHistorySize;

        // Calculate smoothed average
        float smoothedDeltaTime = 0.0f;
        for (float const dt : m_deltaTimeHistory) {
            smoothedDeltaTime += dt;
        }
        smoothedDeltaTime /= static_cast<float>(kDeltaTimeHistorySize);

        // Draw Shadow Map
#ifndef __APPLE__
        ImGui::Begin("Shadow Map Cascades");
        ImGui::Image(m_shadowMapRenderer.GetTextureViewID(m_dbgVizShadowMapCascadeID), ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, -1));
        if (ImGui::Button("<")) {
            --m_dbgVizShadowMapCascadeID;
        }
        ImGui::SameLine();
        ImGui::Text("Cascade ID: %u", m_dbgVizShadowMapCascadeID);
        ImGui::SameLine();
        if (ImGui::Button(">")) {
            ++m_dbgVizShadowMapCascadeID;
        }
        m_dbgVizShadowMapCascadeID %= CSM_NUM_CASCADES;
        ImGui::End();
#endif
        // Draw Logger UI
        m_logger->onScreenLog.Draw("ONFS Log");
        // Draw UI (Tactically)
        ImGui::Text("OpenNFS Engine");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", smoothedDeltaTime * 1000, 1.f / smoothedDeltaTime);
        ImGui::SliderFloat("Time Scale Factor", &userParams.timeScaleFactor, 0, 10);
        ImGui::SliderFloat("Weather Mix Factor", &userParams.weatherMixFactor, 0.5f, 1.f);
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
        ImGui::Checkbox("Draw AI State ", &userParams.drawAIState);
        ImGui::Checkbox("Vroad Viz", &userParams.drawVroad);
        ImGui::Checkbox("CAN Debug", &userParams.drawCAN);
        ImGui::Checkbox("Draw Skydome", &userParams.drawSkydome);
        ImGui::Checkbox("Draw Minimap", &userParams.drawMinimap);
        if (Config::get().hsPhysics) {
            ImGui::Checkbox("NFS4 Physics Debug", &userParams.showNFS4PhysicsDebug);
            ImGui::Checkbox("NFS4 Physics 3D Viz", &userParams.drawNFS4PhysicsVectors);
        }
        ImGui::Text("Camera Target");
        ImGui::SameLine();
        if (ImGui::Button("<")) {
            --m_cameraTargetVehicleID;
        }
        ImGui::SameLine();
        ImGui::Text("Vehicle ID: %u", m_cameraTargetVehicleID);
        ImGui::SameLine();
        if (ImGui::Button(">")) {
            ++m_cameraTargetVehicleID;
        }
        m_cameraTargetVehicleID %= m_numRacers;
        ImGui::NewLine();
        ImGui::ColorEdit3("Sun Atten",
                          reinterpret_cast<float *>(&userParams.sunAttenuation)); // Edit 3 floats representing a colour
        ImGui::SliderFloat("Track Specular Damper", &userParams.trackSpecDamper, 0, 100);
        ImGui::SliderFloat("Track Specular Reflectivity", &userParams.trackSpecReflectivity, 0, 10);
    }

    bool Renderer::_DrawMenuBar(AssetData &loadedAssets) const {
        bool assetChange = false;
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Track")) {
                for (auto &installedNFS : m_nfsAssetList) {
                    if (ImGui::BeginMenu(magic_enum::enum_name(installedNFS.tag).data())) {
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
                    if (ImGui::BeginMenu(magic_enum::enum_name(installedNFS.tag).data())) {
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

    void Renderer::_DrawNFS4PhysicsDebug(std::vector<std::shared_ptr<CarAgent>> const &racers) {
        if (racers[0]->vehicle->physicsModel != PhysicsModel::NFS4_PC) {
            return;
        }

        auto *physics = racers[0]->vehicle->GetNFS4VehiclePhysics();
        auto const &state = physics->GetState();
        auto const &perf = physics->GetPerformanceData();
        auto const &debug = physics->GetDebugData();
        auto &toggles = physics->GetToggles();

        ImGui::SetNextWindowSize(ImVec2(600, 800), ImGuiCond_FirstUseEver);
        ImGui::Begin("NFS4 Vehicle Physics Debug");

        // Gear names for display
        static char const *gearNames[] = {"R", "N", "1", "2", "3", "4", "5", "6"};
        int const gearIdx = static_cast<int>(state.gear);

        // Basic state section
        if (ImGui::CollapsingHeader("Vehicle State", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(2, "state_cols");
            ImGui::Text("Speed: %.1f km/h", physics->GetSpeedKMH());
            ImGui::Text("RPM: %.0f", state.rpm);
            ImGui::Text("Gear: %s", gearNames[gearIdx]);
            ImGui::Text("Redline: %.0f", perf.engineRedlineRPM);
            ImGui::NextColumn();
            ImGui::Text("Throttle: %.2f", state.throttle);
            ImGui::Text("Brake: %.2f", state.brake);
            ImGui::Text("Steering: %.1f", state.currentSteering);
            ImGui::Text("Handbrake: %s", state.handbrakeInput ? "ON" : "OFF");
            ImGui::Columns(1);

            // RPM bar
            float const rpmNorm = state.rpm / perf.engineRedlineRPM;
            ImVec4 rpmColor = rpmNorm > 0.9f ? ImVec4(1, 0, 0, 1) : (rpmNorm > 0.7f ? ImVec4(1, 1, 0, 1) : ImVec4(0, 1, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, rpmColor);
            ImGui::ProgressBar(rpmNorm, ImVec2(-1, 0), "RPM");
            ImGui::PopStyleColor();

            // Throttle/Brake bars
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0, 1, 0, 1));
            ImGui::ProgressBar(state.throttle, ImVec2(-1, 0), "Throttle");
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1, 0, 0, 1));
            ImGui::ProgressBar(state.brake, ImVec2(-1, 0), "Brake");
            ImGui::PopStyleColor();
        }

        // Physics state section
        if (ImGui::CollapsingHeader("Physics State", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Slip Angle: %.4f", state.slipAngle);
            ImGui::Text("Speed XZ: %.2f m/s", state.speedXZ);
            ImGui::Text("G-Transfer: %.4f", state.gTransfer);
            ImGui::Text("Steering Angle: %.4f rad", state.steeringAngle);
            ImGui::Text("Traction Force: %.4f", state.tractionForce);
            ImGui::Text("Lost Grip: %s", state.lostGrip ? "YES" : "NO");
            ImGui::Text("Ground Contact: %s", state.hasContactWithGround ? "YES" : "NO");
            ImGui::Text("Distance Above Ground: %.3f", state.distanceAboveGround);
            ImGui::Text("Handbrake Accum: %d", state.handbrakeAccumulator);
            ImGui::Separator();
            ImGui::Text("Drag: %.6f", debug.drag);
            ImGui::Text("Torque: %.2f Nm", debug.torque);
            ImGui::Text("Slip Angle Factor: %.4f", debug.slipAngleFactor);
            ImGui::Text("Tire Factor: %.6f", debug.tireFactor);
            ImGui::Text("Angular Vel Factor: %.4f", debug.angularVelocityFactor);
            ImGui::Text("Turning Circle Angular Damp: %.4f", debug.turningCircleAngularDamp);
            ImGui::Text("Lateral Velocity Damp: %.4f", debug.lateralVelocityDamp);
            ImGui::Text("Near Stop Decel Factor: %.4f", debug.nearStopDecelFactor);
            ImGui::Text("Airborne Downforce: %.4f", debug.airborneDownforce);
            ImGui::Separator();
            // Status flags
            ImGui::TextColored(debug.preventedSideways ? ImVec4(0, 1, 0, 1) : ImVec4(0.5f, 0.5f, 0.5f, 1),
                              "Prevented Sideways: %s", debug.preventedSideways ? "ACTIVE" : "inactive");
            ImGui::TextColored(debug.appliedNeutralDecel ? ImVec4(0, 1, 0, 1) : ImVec4(0.5f, 0.5f, 0.5f, 1),
                              "Neutral Gear Decel: %s", debug.appliedNeutralDecel ? "ACTIVE" : "inactive");
            ImGui::TextColored(debug.appliedNearStopDecel ? ImVec4(0, 1, 0, 1) : ImVec4(0.5f, 0.5f, 0.5f, 1),
                              "Near Stop Decel: %s", debug.appliedNearStopDecel ? "ACTIVE" : "inactive");
            ImGui::Text("Road Adjust Ang Vel: (%.3f, %.3f, %.3f)",
                        debug.roadAdjustmentAngVel.x(), debug.roadAdjustmentAngVel.y(), debug.roadAdjustmentAngVel.z());
        }

        // Physics feature toggles for debugging
        if (ImGui::CollapsingHeader("Physics Toggles (Debug)", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWrapped("Disable individual physics features to isolate issues:");
            ImGui::Separator();

            ImGui::Columns(2, "toggle_cols");

            // Column 1: Core physics
            ImGui::Text("Core Physics:");
            ImGui::Checkbox("Traction Model", &toggles.enableTractionModel);
            ImGui::Checkbox("Wheel Forces", &toggles.enableWheelForces);
            ImGui::Checkbox("Downforce", &toggles.enableDownforce);

            ImGui::Spacing();
            ImGui::Text("Steering/Handling:");
            ImGui::Checkbox("Turning Circle", &toggles.enableTurningCircle);
            ImGui::Checkbox("Lateral Damping", &toggles.enableLateralDamping);
            ImGui::Checkbox("Prevent Sideways", &toggles.enablePreventSideways);

            ImGui::NextColumn();

            // Column 2: Deceleration and airborne
            ImGui::Text("Deceleration:");
            ImGui::Checkbox("Near Stop Decel", &toggles.enableNearStopDecel);
            ImGui::Checkbox("Neutral Gear Decel", &toggles.enableNeutralGearDecel);

            ImGui::Spacing();
            ImGui::Text("Airborne/Ground:");
            ImGui::Checkbox("Airborne Drag", &toggles.enableAirborneDrag);
            ImGui::Checkbox("Limit Angular Vel", &toggles.enableLimitAngularVelocity);
            ImGui::Checkbox("Adjust To Road", &toggles.enableAdjustToRoad);
            ImGui::Checkbox("Prevent Sinking", &toggles.enablePreventSinking);
            ImGui::Checkbox("Go Airborne", &toggles.enableGoAirborne);

            ImGui::Columns(1);

            ImGui::Separator();
            if (ImGui::Button("Enable All")) {
                toggles.enableTractionModel = true;
                toggles.enableWheelForces = true;
                toggles.enableDownforce = true;
                toggles.enableTurningCircle = true;
                toggles.enableLateralDamping = true;
                toggles.enablePreventSideways = true;
                toggles.enableNearStopDecel = true;
                toggles.enableNeutralGearDecel = true;
                toggles.enableAirborneDrag = true;
                toggles.enableLimitAngularVelocity = true;
                toggles.enableAdjustToRoad = true;
                toggles.enablePreventSinking = true;
                toggles.enableGoAirborne = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Disable All")) {
                toggles.enableTractionModel = false;
                toggles.enableWheelForces = false;
                toggles.enableDownforce = false;
                toggles.enableTurningCircle = false;
                toggles.enableLateralDamping = false;
                toggles.enablePreventSideways = false;
                toggles.enableNearStopDecel = false;
                toggles.enableNeutralGearDecel = false;
                toggles.enableAirborneDrag = false;
                toggles.enableLimitAngularVelocity = false;
                toggles.enableAdjustToRoad = false;
                toggles.enablePreventSinking = false;
                toggles.enableGoAirborne = false;
            }
        }

        // Mesh visibility toggles
        if (ImGui::CollapsingHeader("Mesh Visibility")) {
            auto &car = *racers[0]->vehicle;
            ImGui::Checkbox("Car Body", &car.carBodyModel.enabled);
            ImGui::SameLine();
            ImGui::Checkbox("Left Front Wheel", &car.leftFrontWheelModel.enabled);
            ImGui::Checkbox("Right Front Wheel", &car.rightFrontWheelModel.enabled);
            ImGui::SameLine();
            ImGui::Checkbox("Left Rear Wheel", &car.leftRearWheelModel.enabled);
            ImGui::Checkbox("Right Rear Wheel", &car.rightRearWheelModel.enabled);

            ImGui::Separator();
            if (ImGui::Button("Show All Meshes")) {
                car.carBodyModel.enabled = true;
                car.leftFrontWheelModel.enabled = true;
                car.rightFrontWheelModel.enabled = true;
                car.leftRearWheelModel.enabled = true;
                car.rightRearWheelModel.enabled = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Hide All Meshes")) {
                car.carBodyModel.enabled = false;
                car.leftFrontWheelModel.enabled = false;
                car.rightFrontWheelModel.enabled = false;
                car.leftRearWheelModel.enabled = false;
                car.rightRearWheelModel.enabled = false;
            }
        }

        // Velocity vectors
        if (ImGui::CollapsingHeader("Velocities")) {
            ImGui::Text("Local Velocity: (%.2f, %.2f, %.2f)", debug.localVelocity.x(), debug.localVelocity.y(), debug.localVelocity.z());
            ImGui::Text("Local Ang Vel: (%.4f, %.4f, %.4f)", debug.localAngularVelocity.x(), debug.localAngularVelocity.y(),
                        debug.localAngularVelocity.z());
            ImGui::Text("Ground Orient: (%.3f, %.3f, %.3f)", debug.orientationToGround.x(), debug.orientationToGround.y(),
                        debug.orientationToGround.z());
        }

        // Forces section
        if (ImGui::CollapsingHeader("Forces")) {
            ImGui::Text("Total Force: (%.4f, %.4f, %.4f)", debug.totalForce.x(), debug.totalForce.y(), debug.totalForce.z());
            ImGui::Text("Total Torque: (%.4f, %.4f, %.4f)", debug.totalTorque.x(), debug.totalTorque.y(), debug.totalTorque.z());
        }

        // Per-wheel data
        if (ImGui::CollapsingHeader("Wheel Data", ImGuiTreeNodeFlags_DefaultOpen)) {
            static char const *wheelNames[] = {"FL", "FR", "RL", "RR"};
            if (ImGui::BeginTable("wheels", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Wheel");
                ImGui::TableSetupColumn("Contact");
                ImGui::TableSetupColumn("Susp Len");
                ImGui::TableSetupColumn("Grip");
                ImGui::TableSetupColumn("Traction");
                ImGui::TableSetupColumn("Downforce");
                ImGui::TableSetupColumn("Force (X,Z)");
                ImGui::TableHeadersRow();

                for (int i = 0; i < 4; i++) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", wheelNames[i]);
                    ImGui::TableNextColumn();
                    if (debug.wheelInContact[i]) {
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "YES");
                    } else {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "NO");
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text("%.3f", debug.wheelSuspensionLength[i]);
                    ImGui::TableNextColumn();
                    ImGui::Text("%.3f", debug.wheels[i].grip);
                    ImGui::TableNextColumn();
                    ImGui::Text("%.3f", debug.wheels[i].traction);
                    ImGui::TableNextColumn();
                    ImGui::Text("%.3f", debug.wheels[i].downforce);
                    ImGui::TableNextColumn();
                    ImGui::Text("(%.2f, %.2f)", debug.wheelForces[i].x(), debug.wheelForces[i].z());
                }
                ImGui::EndTable();
            }
        }

        // Performance data
        if (ImGui::CollapsingHeader("Performance Data")) {
            ImGui::Text("Mass: %.1f kg", perf.mass);
            ImGui::Text("Max Velocity: %.1f m/s", perf.maxVelocity);
            ImGui::Text("Lateral Grip Mult: %.2f", perf.lateralGripMultiplier);
            ImGui::Text("Front Grip Bias: %.2f", perf.frontGripBias);
            ImGui::Text("Front Drive Ratio: %.2f", perf.frontDriveRatio);
            ImGui::Text("Front Brake Bias: %.2f", perf.frontBrakeBias);
            ImGui::Text("Understeer Gradient: %.2f", perf.understeerGradient);
            ImGui::Text("Turn Circle Radius: %.1f m", perf.turningCircleRadius);
            ImGui::Text("Downforce Mult: %.4f", perf.downforceMult);
            ImGui::Text("G-Transfer Factor: %.2f", perf.gTransferFactor);
            ImGui::Text("Max Braking Decel: %.1f m/s^2", perf.maxBrakingDeceleration);
            ImGui::Text("Has ABS: %s", perf.hasABS ? "YES" : "NO");
            ImGui::Text("Has Spoiler: %s", perf.hasSpoiler ? "YES" : "NO");
        }

        // Graphs section
        if (ImGui::CollapsingHeader("Graphs", ImGuiTreeNodeFlags_DefaultOpen)) {
            constexpr size_t histSize = OpenNFS::NFS4DebugData::HISTORY_SIZE;
            size_t const offset = debug.historyIndex;

            // Helper to plot circular buffer
            auto plotCircularBuffer = [&](char const *label, std::array<float, histSize> const &data, float scaleMin, float scaleMax) {
                if (ImPlot::BeginPlot(label, ImVec2(-1, 120))) {
                    ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, 0);
                    ImPlot::SetupAxisLimits(ImAxis_Y1, scaleMin, scaleMax, ImPlotCond_Always);

                    // Plot data accounting for circular buffer offset
                    std::array<float, histSize> ordered;
                    for (size_t i = 0; i < histSize; i++) {
                        ordered[i] = data[(offset + i) % histSize];
                    }
                    ImPlot::PlotLine("", ordered.data(), static_cast<int>(histSize));
                    ImPlot::EndPlot();
                }
            };

            plotCircularBuffer("RPM", debug.rpmHistory, 0, perf.engineRedlineRPM * 1.1f);
            plotCircularBuffer("Speed (km/h)", debug.speedHistory, 0, perf.maxVelocity * 4.0f);
            plotCircularBuffer("Throttle/Brake", debug.throttleHistory, 0, 1.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1, 0, 0, 1));
            if (ImPlot::BeginPlot("Brake", ImVec2(-1, 120))) {
                ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, 0);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1.0f, ImPlotCond_Always);
                std::array<float, histSize> ordered;
                for (size_t i = 0; i < histSize; i++) {
                    ordered[i] = debug.brakeHistory[(offset + i) % histSize];
                }
                ImPlot::PlotLine("", ordered.data(), static_cast<int>(histSize));
                ImPlot::EndPlot();
            }
            ImPlot::PopStyleColor();

            plotCircularBuffer("Steering", debug.steeringHistory, -1.0f, 1.0f);
            plotCircularBuffer("Slip Angle", debug.slipAngleHistory, -1.0f, 1.0f);
            plotCircularBuffer("Traction Force", debug.tractionForceHistory, -15.0f, 15.0f);
            plotCircularBuffer("G-Transfer", debug.gTransferHistory, -5.0f, 5.0f);
        }

        // Torque curve visualization
        if (ImGui::CollapsingHeader("Torque Curve")) {
            if (ImPlot::BeginPlot("Engine Torque vs RPM", ImVec2(-1, 200))) {
                ImPlot::SetupAxes("RPM", "Torque (Nm)");
                ImPlot::SetupAxisLimits(ImAxis_X1, 0, 10500);

                std::array<float, 21> rpmValues;
                for (int i = 0; i < 21; i++) {
                    rpmValues[i] = i * 500.0f;
                }
                ImPlot::PlotLine("Torque", rpmValues.data(), perf.torqueCurve.data(), 21);

                // Mark current RPM
                float currentTorque = debug.torque;
                ImPlot::PlotScatter("Current", &state.rpm, &currentTorque, 1);

                ImPlot::EndPlot();
            }
        }

        // Gear ratios
        if (ImGui::CollapsingHeader("Gear Ratios")) {
            if (ImGui::BeginTable("gears", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Gear");
                ImGui::TableSetupColumn("Vel->RPM");
                ImGui::TableSetupColumn("Efficiency");
                ImGui::TableSetupColumn("Shift RPM");
                ImGui::TableHeadersRow();

                for (int i = 0; i < perf.maxGear + 1; i++) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", gearNames[i]);
                    ImGui::TableNextColumn();
                    ImGui::Text("%.3f", perf.gearVelocityToRPM[i]);
                    ImGui::TableNextColumn();
                    ImGui::Text("%.3f", perf.gearEfficiency[i]);
                    ImGui::TableNextColumn();
                    ImGui::Text("%.0f", perf.shiftBlipRPM[i]);
                }
                ImGui::EndTable();
            }
        }

        ImGui::End();
    }

    void Renderer::Shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        glfwTerminate();
    }
} // namespace OpenNFS
