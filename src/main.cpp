//
//  main.cpp
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 27/01/2015.
//
#include <cstdlib>
#include <string>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <examples/opengl3_example/imgui_impl_glfw_gl3.h>
#include <set>

#define TINYOBJLOADER_IMPLEMENTATION
// Source
#include "Util/Utils.h"
#include "Scene/Camera.h"
#include "Loaders/nfs_loader.h"
#include "Loaders/trk_loader.h"
#include "Loaders/music_loader.h"
#include "Shaders/TrackShader.h"
#include "Shaders/CarShader.h"
#include "Shaders/BillboardShader.h"
#include "Physics/Physics.h"
#include "Physics/Car.h"

GLFWwindow *window;

using namespace ImGui;

bool init_opengl() {
    // Initialise GLFW
    ASSERT(glfwInit(), "GLFW Init failed.\n");

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "OpenNFS3", nullptr, nullptr);

    if (window == nullptr) {
        fprintf(stderr, "Failed to create a GLFW window.\n");
        getchar();
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return false;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera (when culling is enabled)
    glFrontFace(GL_CW);
    glEnable(GL_BACK);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return true;
}

void newFrame(bool &window_active) {
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

void initDirectories(){
    if(!(boost::filesystem::exists(CAR_PATH))){
        boost::filesystem::create_directories(CAR_PATH);
    }
    if(!(boost::filesystem::exists(TRACK_PATH))){
        boost::filesystem::create_directories(TRACK_PATH);
    }
}

int main(int argc, char **argv) {
    std::cout << "----------- OpenNFS3 v0.01 -----------" << std::endl;
    ASSERT(init_opengl(), "OpenGL init failed.");

    /*------ ASSET LOAD ------*/
    initDirectories();
    NFS_Loader nfs_loader("../resources/cars/car.viv");
    //Load Car data from unpacked NFS files
    Car car = Car(nfs_loader);

    //Load Track Data`
    //NFS2::TRACK *track = NFS2::trk_loader("../resources/NFS2/GAMEDATA/TRACKS/SE/TR02");
    NFS3::TRACK *track = NFS3::trk_loader("../resources/NFS3/gamedata/tracks/trk003/tr03");
	//Load Music
	//MusicLoader musicLoader("F:\\NFS3\\nfs3_modern_base_eng\\gamedata\\audio\\pc\\hometech");

    /*------- BULLET --------*/
    /*Physics physicsEngine;
    physicsEngine.registerTrack(track->track_blocks);
    physicsEngine.registerVehicle(&car);*/

    /*------- ImGui -------*/
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();

    // Create and compile our GLSL programs from the shaders
    TrackShader trackShader;
    CarShader carShader;
    BillboardShader billboardShader;

    Camera mainCamera(glm::vec3(98.46, 3.98, 0), 45.0f, 4.86f, -0.21f);
    Light cameraLight(mainCamera.position, glm::vec3(1, 1, 1));

    // Data used for culling
    std::vector<TrackBlock> activeTrackBlocks;
    glm::vec3 oldWorldPosition(0, 0, 0);
    int closestBlockID = 0;

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

    while (!glfwWindowShouldClose(window)) {
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        newFrame(window_active);
        // Compute the MVP matrix from keyboard and mouse input
        glm::vec3 carCam = car.car_models[0].position;
        carCam.y += 0.2;
        //carCam.z -= 1;
        if (!ImGui::GetIO().MouseDown[1]) {
            mainCamera.position = carCam;
        }
        mainCamera.computeMatricesFromInputs(window_active, ImGui::GetIO());
        glm::mat4 ProjectionMatrix = mainCamera.ProjectionMatrix;
        glm::mat4 ViewMatrix = mainCamera.ViewMatrix;
        glm::vec3 worldPosition = mainCamera.position;
        //physicsEngine.mydebugdrawer.SetMatrices(ViewMatrix, ProjectionMatrix);

        //TODO: Refactor to controller class?
        if (window_active && !ImGui::GetIO().MouseDown[1]) {
            car.applyAccelerationForce(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
            car.applyBrakingForce(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
            car.applySteeringRight(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
            car.applySteeringLeft(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                car.toggleReverse();
            }
        }

        // Basic Geometry Cull
        if ((oldWorldPosition.x != worldPosition.x) && (oldWorldPosition.z != worldPosition.z)) {
            cameraLight.position = worldPosition;
            float lowestDistanceSqr = FLT_MAX;
            //Primitive Draw distance
            for (auto &track_block :  track->track_blocks) {
                glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0)));
                glm::vec3 position = orientation *
                                     glm::vec3(track_block.center.x / 10, track_block.center.y / 10,
                                               track_block.center.z / 10);
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
        //physicsEngine.stepSimulation(mainCamera.deltaTime);


        carShader.use();
        for (auto &car_model : car.car_models) {
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
            for (auto &track_block_model : active_track_Block.track) {
                track_block_model.update();
                trackShader.loadMatrices(ProjectionMatrix, ViewMatrix, track_block_model.ModelMatrix);
                trackShader.loadSpecular(trackSpecDamper, trackSpecReflectivity);
                if (active_track_Block.lights.size() > 0) {
                    trackShader.loadLight(active_track_Block.lights[0]);
                }
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

        /*if (physics_debug_view)
            physicsEngine.getDynamicsWorld()->debugDrawWorld();*/

        // Draw UI (Tactically)
        static float f = 0.0f;
        ImGui::Text("NFS3 Engine");
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
            car.resetCar();
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

        for (auto &mesh : car.car_models) {
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

    free(track);
    // Cleanup VBOs and shaders
    carShader.cleanup();
    trackShader.cleanup();
    ImGui_ImplGlfwGL3_Shutdown();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
