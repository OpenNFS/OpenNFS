//
// Created by SYSTEM on 19/07/2018.
//


#include "Renderer.h"


Renderer::Renderer(GLFWwindow *gl_window, shared_ptr<ONFSTrack> current_track, shared_ptr<Car> current_car) : carShader(
        current_car) {
    window = gl_window;
    track = current_track;
    car = current_car;

    mainCamera = Camera(glm::vec3(track->track_blocks[0].center.x / 10, track->track_blocks[0].center.y / 10,
                                  track->track_blocks[0].center.z / 10), 45.0f, 4.86f, -0.21f, window);
    mainCamera.generateSpline(track->track_blocks);

    cameraLight = Light(mainCamera.position, glm::vec3(1, 1, 1));

    // Generate the collision meshes
    physicsEngine.registerTrack(track->track_blocks);
    physicsEngine.registerVehicle(car);


    /*------- ImGui -------*/
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();
}

void Renderer::render() {
    /*------- UI -------*/
    ImVec4 clear_color = ImVec4(119 / 255.0f, 197 / 255.0f, 252 / 255.0f, 1.0f);
    ImVec4 car_color = ImVec4(247 / 255.0f, 203 / 255.0f, 32 / 255.0f, 1.0f);
    ImVec4 test_light_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    float carSpecReflectivity = 1;
    float carSpecDamper = 10;
    float trackSpecReflectivity = 1;
    float trackSpecDamper = 10;
    int blockDrawDistance = 15;
    bool window_active = true;
    bool physics_debug_view = false;
    bool use_classic_graphics = true;
    bool attach_cam_to_hermite = false;
    bool use_nb_data = true;

    glm::vec3 oldWorldPosition(0, 0, 0);

    // Shit
    std::vector<Light> camlights;
    camlights.push_back(cameraLight);

    car->setPosition(mainCamera.position);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        newFrame(window_active);

        if (attach_cam_to_hermite) {
            mainCamera.useSpline();
        }

        if (!ImGui::GetIO().MouseDown[1]) {
            //glm::vec3 carCam = car->car_models[0].position;
            //carCam.y += 0.2;
            //carCam.z -= 1;
            //mainCamera.position = carCam;
            // Camera position along the spline
        }
        // Compute the MVP matrix from keyboard and mouse input
        mainCamera.computeMatricesFromInputs(window_active, ImGui::GetIO());

        glm::mat4 ProjectionMatrix = mainCamera.ProjectionMatrix;
        glm::mat4 ViewMatrix = mainCamera.ViewMatrix;
        glm::vec3 worldPosition = mainCamera.position;
        physicsEngine.mydebugdrawer.SetMatrices(ViewMatrix, ProjectionMatrix);

        //TODO: Refactor to controller class?
        if (window_active && !ImGui::GetIO().MouseDown[1]) {
            car->applyAccelerationForce(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
            car->applyBrakingForce(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
            car->applySteeringRight(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
            car->applySteeringLeft(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                car->toggleReverse();
            }
        }

        // Step the physics simulation
        physicsEngine.stepSimulation(mainCamera.deltaTime);

        std::vector<int> activeTrackBlockIDs = CullTrackBlocks(oldWorldPosition, worldPosition, blockDrawDistance,
                                                               use_nb_data);

        // Render the per-trackblock data
        for (int activeBlk_Idx = 0; activeBlk_Idx < activeTrackBlockIDs.size(); ++activeBlk_Idx) {
            TrackBlock active_track_Block = track->track_blocks[activeTrackBlockIDs[activeBlk_Idx]];
            std::vector<Light> contributingLights = active_track_Block.lights;
            // TODO: Merge lighting contributions across track block, must use a smarter Track structure

            trackShader.use();
            for (auto &track_block_model : active_track_Block.objects) {
                track_block_model.update();
                trackShader.loadMatrices(ProjectionMatrix, ViewMatrix, track_block_model.ModelMatrix);
                trackShader.loadSpecular(trackSpecDamper, trackSpecReflectivity);
                if (contributingLights.size() > 0) {
                    trackShader.loadLights(contributingLights);
                } else {
                    trackShader.loadLights(camlights);
                }
                trackShader.bindTrackTextures(track_block_model, track->texture_gl_mappings);
                trackShader.setClassic(use_classic_graphics);
                track_block_model.render();
            }
            trackShader.unbind();
        }

        // Render the global data, animations go here.
        trackShader.use();
        for (auto &global_object : track->global_objects) {
            global_object.update();
            trackShader.loadMatrices(ProjectionMatrix, ViewMatrix, global_object.ModelMatrix);
            trackShader.loadSpecular(trackSpecDamper, trackSpecReflectivity);
            trackShader.loadLights(camlights);
            trackShader.bindTrackTextures(global_object, track->texture_gl_mappings);
            trackShader.setClassic(use_classic_graphics);
            global_object.render();
        }
        trackShader.unbind();

        // Render the Car
        carShader.use();
        for (auto &car_model : car->car_models) {
            carShader.loadMatrices(ProjectionMatrix, ViewMatrix, car_model.ModelMatrix);
            carShader.loadSpecular(car_model.specularDamper, car_model.specularReflectivity, car_model.envReflectivity);
            carShader.loadCarColor(
                    car_model.envReflectivity > 0.4 ? glm::vec3(car_color.x, car_color.y, car_color.z) : glm::vec3(1, 1,
                                                                                                                   1));
            carShader.loadLight(cameraLight);
            carShader.loadCarTexture();
            car_model.render();
        }
        carShader.unbind();


        for (auto &track_block_id : activeTrackBlockIDs) {
            billboardShader.use();
            // Render the lights far to near
            for (auto &light : std::vector<Light>(track->track_blocks[track_block_id].lights.rbegin(),
                                                  track->track_blocks[track_block_id].lights.rend())) {
                light.update();
                billboardShader.loadMatrices(ProjectionMatrix, ViewMatrix, light.ModelMatrix);
                billboardShader.loadLight(light);
                light.render();
            }
            billboardShader.unbind();
        }

        // PICKING IS DONE HERE
        // (Instead of picking each frame if the mouse button is down,
        // you should probably only check if the mouse button was just released)
        if (ImGui::GetIO().MouseReleased[0]) {
            glm::vec3 out_origin;
            glm::vec3 out_direction;
            ScreenPosToWorldRay(
                    1024 / 2, 768 / 2,
                    1024, 768,
                    ViewMatrix,
                    ProjectionMatrix,
                    out_origin,
                    out_direction
            );
            glm::vec3 out_end = out_origin + out_direction * 1000.0f;
            btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(out_origin.x, out_origin.y, out_origin.z),
                                                                   btVector3(out_end.x, out_end.y, out_end.z));
            physicsEngine.getDynamicsWorld()->rayTest(btVector3(out_origin.x, out_origin.y, out_origin.z),
                                                      btVector3(out_end.x, out_end.y, out_end.z), RayCallback);
            if (RayCallback.hasHit()) {
                std::ostringstream oss;
                // This callback is only going to work on the Car, which sucks ass
                //oss << "mesh " << static_cast<Car*>(RayCallback.m_collisionObject->getUserPointer())->name;
                std::cout << oss.str() << std::endl;
            } else {

            }
        }

        if (physics_debug_view)
            physicsEngine.getDynamicsWorld()->debugDrawWorld();

        // Draw UI (Tactically)
        static float f = 0.0f;
        DrawMenuBar();
        ImGui::Text("OpenNFS Engine");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::Checkbox("Bullet Debug View", &physics_debug_view);
        ImGui::Checkbox("Classic Graphics", &use_classic_graphics);
        ImGui::Checkbox("Hermite Curve Cam", &attach_cam_to_hermite);
        std::stringstream world_position_string;
        world_position_string << "X " << std::to_string(worldPosition.x) << " Y " << std::to_string(worldPosition.y)
                              << " Z " << std::to_string(worldPosition.z) << " H: "
                              << std::to_string(mainCamera.horizontalAngle) << " V: "
                              << std::to_string(mainCamera.verticalAngle);
        ImGui::Text(world_position_string.str().c_str());
        ImGui::Text(("Block ID: " + std::to_string(closestBlockID)).c_str());

        if (ImGui::Button("Reset View")) {
            mainCamera.resetView();
        };
        ImGui::SameLine(0, -1.0f);
        if (ImGui::Button("Reset Car")) {
            car->resetCar();
        };
        ImGui::NewLine();
        ImGui::SameLine(0, 0.0f);
        ImGui::SliderInt("Draw Dist", &blockDrawDistance, 0, track->nBlocks);
        ImGui::Checkbox("NBData", &use_nb_data);
        ImGui::NewLine();
        ImGui::ColorEdit3("Clear Colour", (float *) &clear_color); // Edit 3 floats representing a color
        ImGui::ColorEdit3("Testing Light Colour", (float *) &test_light_color);
        ImGui::ColorEdit3("Car Colour", (float *) &car_color);
        ImGui::SliderFloat("Car Specular Damper", &carSpecDamper, 0, 100);
        ImGui::SliderFloat("Car Specular Reflectivity", &carSpecReflectivity, 0, 10);
        ImGui::SliderFloat("Track Specular Damper", &trackSpecDamper, 0, 100);
        ImGui::SliderFloat("Track Specular Reflectivity", &trackSpecReflectivity, 0, 10);

        if (ImGui::TreeNode("Car Models")) {
            for (auto &mesh : car->car_models) {
                ImGui::Checkbox((mesh.m_name + std::to_string(mesh.id)).c_str(), &mesh.enabled);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Track Blocks")) {
            for (auto &track_block :  track->track_blocks) {
                char label[32];
                sprintf(label, "Track Block %d", track_block.block_id);

                ImGui::PushID(label);
                if (ImGui::TreeNode(label)) {
                    if (ImGui::TreeNode("Objects")) {
                        for (auto &block_model : track_block.objects) {
                            ImGui::PushID(block_model.id);
                            if (ImGui::TreeNode(block_model.m_name.c_str())) {
                                ImGui::Checkbox("Enabled", &block_model.enabled);
                                ImGui::TreePop();
                            }
                            ImGui::PopID();

                        }
                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Lights")) {
                        for (auto &block_light : track_block.lights) {
                            char label[32];
                            sprintf(label, "Light %s %d", block_light.m_name.c_str(), block_light.type);
                            if (ImGui::TreeNode(label)) {
                                ImGui::Checkbox("Enabled", &block_light.enabled);
                                ImGui::Text("%f %f %f", block_light.position.x, block_light.position.y,
                                            block_light.position.z);
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }


        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

void Renderer::SetCulling(bool toCull) {
    if (toCull) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void Renderer::DrawDebugCube(glm::vec3 position) {
    float lightSize = 0.5;
    glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
    glm::vec3 position_min =
            orientation * glm::vec3(position.x - lightSize, position.y - lightSize, position.z - lightSize);
    glm::vec3 position_max =
            orientation * glm::vec3(position.x + lightSize, position.y + lightSize, position.z + lightSize);
    btVector3 colour = btVector3(0, 0, 0);
    physicsEngine.mydebugdrawer.drawBox(btVector3(position_min.x, position_min.y, position_min.z),
                                        btVector3(position_max.x, position_max.y, position_max.z), colour);
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
            glm::vec3 position = glm::vec3(track_block.center.x / 10, track_block.center.y / 10,
                                           track_block.center.z / 10);
            float distanceSqr = glm::length2(glm::distance(worldPosition, position));
            if (distanceSqr < lowestDistanceSqr) {
                closestBlockID = track_block.block_id;
                lowestDistanceSqr = distanceSqr;
            }
        }

        // If we have an NFS3 track loaded, use the provided neighbour data to work out which blocks to render
        if (track->tag == NFS_3 && useNeighbourData) {
            for (int i = 0; i < 300; ++i) {
                if (boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(track->trackData)->trk[closestBlockID].nbdData[i].blk ==
                    -1) {
                    break;
                } else {
                    activeTrackBlockIds.emplace_back(boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(
                            track->trackData)->trk[closestBlockID].nbdData[i].blk);
                }
            }
        } else {
            // Use a draw distance value to return closestBlock +- drawDistance inclusive blocks
            int wrapBlocks = 0;
            for (int block_Idx = closestBlockID - blockDrawDistance;
                 block_Idx < closestBlockID + blockDrawDistance; ++block_Idx) {
                if (block_Idx < 0) {
                    int activeBlock =
                            ((int) track->track_blocks.size() + (closestBlockID - blockDrawDistance)) + wrapBlocks++;
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

void Renderer::newFrame(bool &window_active) {
    glfwPollEvents();
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Detect a click on the 3D Window by detecting a click that isn't on ImGui
    window_active = window_active ? window_active : (
            (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) &&
            (!ImGui::GetIO().WantCaptureMouse));
    if (!window_active) {
        ImGui::GetIO().MouseDrawCursor = false;
    }
    ImGui_ImplGlfwGL3_NewFrame();
}
