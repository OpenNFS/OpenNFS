#include "Renderer.h"

#include <backends/imgui_impl_opengl3.h>

namespace OpenNFS {
    Renderer::Renderer(std::shared_ptr<GLFWwindow> const &window,
                       std::shared_ptr<Logger> const &onfsLogger,
                       std::vector<NfsAssetList> const &installedNFS,
                       Track const &currentTrack,
                       std::shared_ptr<BulletDebugDrawer> const &debugDrawer)
        : m_window(window), m_logger(onfsLogger), m_nfsAssetList(installedNFS), m_track(currentTrack), m_debugRenderer(debugDrawer) {
        this->_InitialiseIMGUI();
        LOG(DEBUG) << "Renderer Initialised";
    }

    std::shared_ptr<GLFWwindow> Renderer::InitOpenGL(uint32_t const resolutionX,
                                                     uint32_t const resolutionY,
                                                     std::string const &windowName) {
        // Initialise GLFW
        CHECK_F(glfwInit() == GLFW_TRUE, "GLFW Init failed.\n");
        glfwSetErrorCallback(&Renderer::GlfwError);

        // glfwWindowHint(GLFW_SAMPLES, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods

        auto window = std::shared_ptr<GLFWwindow>(glfwCreateWindow(resolutionX, resolutionY, windowName.c_str(), nullptr, nullptr),
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
        // Set the mouse at the center of the screen
        glfwPollEvents();
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

        Config::get().windowSizeX = resolutionX;
        Config::get().windowSizeY = resolutionY;

        return window;
    }

    bool Renderer::Render(float const totalTime,
                          BaseCamera const &activeCamera,
                          HermiteCamera const &hermiteCamera,
                          GlobalLight const *activeLight,
                          ParamData &userParams,
                          AssetData &loadedAssets,
                          std::vector<std::shared_ptr<CarAgent>> const &racers,
                          std::optional<Entity *> const targetedEntity) {
        bool newAssetSelected = false;
        m_numRacers = racers.size();

        // Perform frustum culling to get visible entities, from perspective of active camera
        auto const [visibleEntities, visibleLights] = _FrustumCull(m_track, activeCamera, activeLight, userParams);

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
                m_debugRenderer.DrawCarRaycasts(racer->vehicle);
            }
        }

        if (userParams.drawCAN) {
            m_debugRenderer.DrawCameraAnimation(m_track);
        }

        // Render the environment
        m_shadowMapRenderer.Render(userParams.nearPlane, userParams.farPlane, activeLight, m_track.textureArrayID, visibleEntities, racers);
        if (userParams.drawSkydome) {
            m_skyRenderer.Render(activeCamera, activeLight, totalTime);
        }
        m_trackRenderer.Render(racers, activeCamera, m_track.textureArrayID, visibleEntities, visibleLights, userParams,
                               m_shadowMapRenderer.GetTextureID(), 0.5f);
        m_trackRenderer.RenderLights(activeCamera, visibleLights);
        m_debugRenderer.Render(activeCamera);
        m_miniMapRenderer.Render(m_track, racers);

        // Render the Car and racers
        for (auto &racer : racers) {
            m_carRenderer.Render(racer->vehicle, activeCamera);
        }

        if (this->_DrawMenuBar(loadedAssets)) {
            newAssetSelected = true;
        }

        if (targetedEntity.has_value()) {
            _DrawMetadata(targetedEntity.value());
        }

        // Render the Debug UI
        this->_DrawDebugUI(userParams, activeCamera);

        glfwSwapBuffers(m_window.get());

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

    VisibleSet Renderer::_FrustumCull(Track const &track,
                                      BaseCamera const &camera,
                                      GlobalLight const *globalLight,
                                      ParamData const &userParams) {
        VisibleSet visibleSet;
        // The sun/moon should always contribute
        visibleSet.lights.push_back(globalLight);

        if (userParams.useFrustumCull) {
            // Perform frustum culling on the current camera, on local trackblocks
            for (auto &collision : track.cullTree.queryOverlaps(camera.viewFrustum)) {
                auto entity{std::dynamic_pointer_cast<Entity>(collision)};
                if (entity->type == EntityType::LIGHT) {
                    visibleSet.lights.emplace_back(*entity);
                } else {
                    visibleSet.entities.emplace_back(entity);
                }
            }
        } else {
            std::vector<uint32_t> const localTrackBlockIDs{_GetLocalTrackBlockIDs(track, camera)};
            for (auto const &localTrackBlockID : localTrackBlockIDs) {
                visibleSet.entities.insert(visibleSet.entities.end(), track.perTrackblockEntities.at(localTrackBlockID).begin(),
                                           track.perTrackblockEntities.at(localTrackBlockID).end());
            }
        }

        return visibleSet;
    }

    std::vector<uint32_t> Renderer::_GetLocalTrackBlockIDs(Track const &track, BaseCamera const &camera) {
        std::vector<uint32_t> activeTrackBlockIds;
        uint32_t nearestBlockID = 0;

        float lowestDistance = FLT_MAX;

        // Get the closest track block to camera position
        for (auto &trackblock : track.trackBlocks) {
            if (float const distance = glm::distance(camera.position, trackblock.position); distance < lowestDistance) {
                nearestBlockID = trackblock.id;
                lowestDistance = distance;
            }
        }

        TrackBlock const &nearestTrackBlock{track.trackBlocks[nearestBlockID]};
        // Use the provided neighbour data to work out which blocks to render if there is any
        if (!nearestTrackBlock.neighbourIds.empty()) {
            activeTrackBlockIds = nearestTrackBlock.neighbourIds;
        } else {
            constexpr int32_t kBlockDistance{15};
            // Use a draw distance value to return closestBlock +- kBlockDrawDistance inclusive blocks
            for (int32_t trackblockIdx{static_cast<int32_t>(nearestBlockID) - kBlockDistance};
                 trackblockIdx < nearestBlockID + kBlockDistance; ++trackblockIdx) {
                uint32_t activeBlock = trackblockIdx < 0 ? (static_cast<uint32_t>(track.trackBlocks.size()) + trackblockIdx)
                                                         : (trackblockIdx % static_cast<uint32_t>(track.trackBlocks.size()));
                activeTrackBlockIds.emplace_back(activeBlock);
            }
        }

        return activeTrackBlockIds;
    }

    void Renderer::_InitialiseIMGUI() const {
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
        std::string const glVersion = "#version " + ONFS_GL_VERSION;
        ImGui_ImplOpenGL3_Init(glVersion.c_str());
        ImGui::StyleColorsDark();
    }

    void Renderer::_DrawMetadata(Entity const *targetEntity) {
        auto const track_entity{dynamic_cast<TrackEntity const *>(targetEntity)};
        if (track_entity == nullptr) {
            return;
        }
        ImGui::Begin("Engine Entity");
        ImGui::Text("%s", magic_enum::enum_name(track_entity->type).data());
        if (track_entity->entityID != -1) {
            ImGui::Text("ID: %d", track_entity->entityID);
        }
        ImGui::Separator();

        // Traverse the loader structures and print pretty with IMGUI
        switch (track_entity->type) {
        case EntityType::LIGHT: {
            auto const targetBaseLight = targetEntity->baseLight;
            ImVec4 lightColour(targetBaseLight->colour.x, targetBaseLight->colour.y, targetBaseLight->colour.z, targetBaseLight->colour.w);
            ImVec4 lightAttenuation(targetBaseLight->attenuation.x, targetBaseLight->attenuation.y, targetBaseLight->attenuation.z, 0.0f);
            // Colour, type, attenuation, position and NFS unknowns
            ImGui::ColorEdit4("Light Colour", reinterpret_cast<float *>(&lightColour)); // Edit 3 floats representing a color
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
        ImGui::Text("Object Flags: %d", targetEntity->flags);
        ImGui::Text("Collidable: %s", targetEntity->collidable ? "Yes" : "No");
        ImGui::Text("Dynamic: %s", targetEntity->dynamic ? "Yes" : "No");
        ImGui::End();
    }

    void Renderer::_DrawDebugUI(ParamData &userParams, BaseCamera const &camera) {
        // Draw Shadow Map
        ImGui::Begin("Shadow Map");
        ImGui::Image(m_shadowMapRenderer.GetTextureID(), ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, -1));
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
        ImGui::Checkbox("Draw Skydome", &userParams.drawSkydome);
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
                          reinterpret_cast<float *>(&userParams.sunAttenuation)); // Edit 3 floats representing a color
        ImGui::SliderFloat("Track Specular Damper", &userParams.trackSpecDamper, 0, 100);
        ImGui::SliderFloat("Track Specular Reflectivity", &userParams.trackSpecReflectivity, 0, 10);

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(m_window.get(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

    Renderer::~Renderer() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
} // namespace OpenNFS
