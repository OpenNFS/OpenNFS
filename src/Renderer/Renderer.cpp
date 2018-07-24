//
// Created by SYSTEM on 19/07/2018.
//


#include "Renderer.h"


Renderer::Renderer(GLFWwindow *gl_window, shared_ptr<ONFSTrack> current_track, shared_ptr<Car> current_car) {
    window = gl_window;
    track = current_track;
    car = current_car;
    mainCamera = Camera(glm::vec3(98.46, 3.98, 0), 45.0f, 4.86f, -0.21f, window);
    cameraLight = Light(mainCamera.position, glm::vec3(1, 1, 1));
    carShader = CarShader(car);

    // Generate the collision meshes
    physicsEngine.registerTrack(track->track_blocks);
    physicsEngine.registerVehicle(car);

    std::vector<glm::vec3> cameraPoints;
    for(auto &track_block : track->track_blocks){
        glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
        glm::vec3 position = orientation * glm::vec3(track_block.center.x / 10, track_block.center.y / 10, track_block.center.z / 10);
        cameraPoints.emplace_back(position);
    }
    cameraSpline = HermiteCurve(cameraPoints, 0.5, 0.0f);
    loopTime = cameraSpline.points.size() * 100;

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
    float totalTime = 1;
    while (!glfwWindowShouldClose(window)) {
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        newFrame(window_active);
        // Compute the MVP matrix from keyboard and mouse input
        if (!ImGui::GetIO().MouseDown[1]) {
            //glm::vec3 carCam = car->car_models[0].position;
            //carCam.y += 0.2;
            //carCam.z -= 1;
            //mainCamera.position = carCam;
            // Camera position along the spline
            float tmod = mainCamera.deltaTime > 0 ? ((int) mainCamera.deltaTime % loopTime ) / loopTime : 1;
            totalTime+=mainCamera.deltaTime;
            tmod = (totalTime) / (loopTime/200);
            mainCamera.position = cameraSpline.getPointAt( tmod );
        }
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
                car.get()->toggleReverse();
            }
        }

        // Basic Geometry Cull
        if ((oldWorldPosition.x != worldPosition.x) && (oldWorldPosition.z != worldPosition.z)) {
            cameraLight.position = worldPosition;
            float lowestDistanceSqr = FLT_MAX;
            //Primitive Draw distance
            for (auto &track_block :  track->track_blocks) {
                glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
                glm::vec3 position = orientation * glm::vec3(track_block.center.x / 10, track_block.center.y / 10, track_block.center.z / 10);
                float distanceSqr = glm::length2(glm::distance(worldPosition, glm::vec3(position.x, position.y, position.z)));
                if (distanceSqr < lowestDistanceSqr) {
                    closestBlockID = track_block.block_id;
                    lowestDistanceSqr = distanceSqr;
                }
            }
            int frontBlock = closestBlockID <  track->track_blocks.size() - blockDrawDistance ? closestBlockID + blockDrawDistance :  track->track_blocks.size();
            int backBlock = closestBlockID - blockDrawDistance > 0 ? closestBlockID - blockDrawDistance : 0;
            std::vector<TrackBlock>::const_iterator first =  track->track_blocks.begin() + backBlock;
            std::vector<TrackBlock>::const_iterator last =  track->track_blocks.begin() + frontBlock;
            activeTrackBlocks = std::vector<TrackBlock>(first, last);
            oldWorldPosition = worldPosition;
        }

        // Step the physics simulation
        physicsEngine.stepSimulation(mainCamera.deltaTime);

        carShader.use();
        for (auto &car_model : car->car_models) {
            carShader.loadMatrices(ProjectionMatrix, ViewMatrix, car_model.ModelMatrix);
            carShader.loadSpecular(car_model.specularDamper, car_model.specularReflectivity, car_model.envReflectivity);
            carShader.loadCarColor(car_model.envReflectivity > 0.4 ? glm::vec3(car_color.x, car_color.y, car_color.z) : glm::vec3(1, 1, 1));
            carShader.loadLight(cameraLight);
            carShader.loadCarTexture();
            car_model.render();
        }
        carShader.unbind();

        for (auto &active_track_Block : activeTrackBlocks) {
            trackShader.use();
            for (auto &track_block_model : active_track_Block.objects) {
                track_block_model.update();
                trackShader.loadMatrices(ProjectionMatrix, ViewMatrix, track_block_model.ModelMatrix);
                trackShader.loadSpecular(trackSpecDamper, trackSpecReflectivity);
                //if (active_track_Block.lights.size() > 0) {
                //    trackShader.loadLight(active_track_Block.lights[0]);
                //}
                trackShader.bindTrackTextures(track_block_model, track->texture_gl_mappings);
                track_block_model.render();
            }
            trackShader.unbind();

            // billboardShader.use();
            // for (auto &light : active_track_Block.lights) {
            //     light.update();
            //     billboardShader.loadMatrices(ProjectionMatrix, ViewMatrix, light.ModelMatrix);
            //     billboardShader.loadLight(light);
            //     light.render();
            // }
            // billboardShader.unbind();
        }

        if (physics_debug_view)
            //physicsEngine.getDynamicsWorld()->debugDrawWorld();

            // Draw UI (Tactically)
            static float f = 0.0f;


        DrawMenuBar();
        ImGui::Text("OpenNFS Engine");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::Checkbox("Bullet Debug View", &physics_debug_view);
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
        ImGui::SliderInt("Draw Distance", &blockDrawDistance, 0, track->nBlocks);
        ImGui::ColorEdit3("Clear Colour", (float *) &clear_color); // Edit 3 floats representing a color
        ImGui::ColorEdit3("Testing Light Colour", (float *) &test_light_color);
        ImGui::ColorEdit3("Car Colour", (float *) &car_color);
        ImGui::SliderFloat("Car Specular Damper", &carSpecDamper, 0, 100);
        ImGui::SliderFloat("Car Specular Reflectivity", &carSpecReflectivity, 0, 10);
        ImGui::SliderFloat("Track Specular Damper", &trackSpecDamper, 0, 100);
        ImGui::SliderFloat("Track Specular Reflectivity", &trackSpecReflectivity, 0, 10);

        for (auto &mesh : car->car_models) {
            ImGui::Checkbox((mesh.m_name + std::to_string(mesh.id)).c_str(),
                            &mesh.enabled);      // Edit bools storing model draw state
        }
        if (ImGui::TreeNode("Track Blocks")) {
            for (auto &track_block :  track->track_blocks) {
                if (ImGui::TreeNode((void *) track_block.block_id, "Track Block %d", track_block.block_id)) {
                    for (auto &block_model : track_block.objects) {
                        if (ImGui::TreeNode((void *) block_model.id, "%s %d", block_model.m_name.c_str(),
                                            block_model.id)) {
                            ImGui::Checkbox("Enabled", &block_model.enabled);
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
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

void Renderer::DrawMenuBar(){
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O")) {}
                if (ImGui::BeginMenu("Open Recent"))
                {
                    ImGui::MenuItem("fish_hat.c");
                    ImGui::MenuItem("fish_hat.inl");
                    ImGui::MenuItem("fish_hat.h");
                    if (ImGui::BeginMenu("More.."))
                    {
                        ImGui::MenuItem("Hello");
                        ImGui::MenuItem("Sailor");
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Options"))
                {
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
            if (ImGui::BeginMenu("Edit"))
            {
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
