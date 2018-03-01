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
#include <btBulletDynamicsCommon.h>
#include <examples/opengl3_example/imgui_impl_glfw_gl3.h>
#include <set>

#define TINYOBJLOADER_IMPLEMENTATION
// Source
#include "Scene/Camera.h"
#include "nfs_loader.h"
#include "trk_loader.h"
#include "Shaders/TrackShader.h"
#include "Shaders/CarShader.h"
#include "Util/Assert.h"

GLFWwindow *window;

using namespace ImGui;

class BulletDebugDrawer_DeprecatedOpenGL : public btIDebugDraw {
public:
    void SetMatrices(glm::mat4 pViewMatrix, glm::mat4 pProjectionMatrix) {
        glUseProgram(0); // Use Fixed-function pipeline (no shaders)
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&pViewMatrix[0][0]);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&pProjectionMatrix[0][0]);
    }

    virtual void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
        glColor3f(color.x(), color.y(), color.z());
        glBegin(GL_LINES);
        glVertex3f(from.x(), from.y(), from.z());
        glVertex3f(to.x(), to.y(), to.z());
        glEnd();
    }

    virtual void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) {}

    virtual void reportErrorWarning(const char *) {}

    virtual void draw3dText(const btVector3 &, const char *) {}

    virtual void setDebugMode(int p) {
        m = p;
    }

    int getDebugMode(void) const { return 3; }

    int m;
};

bool init_opengl() {
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "GLFW Init failed.\n");
        getchar();
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

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

    // Cull triangles which normal is not towards the camera
    glEnable(GL_FRONT);
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

int main(int argc, const char *argv[]) {
    std::cout << "----------- OpenNFS3 v0.01 -----------" << std::endl;
    ASSERT(init_opengl(), "OpenGL init failed.");

    NFS_Loader nfs_loader("../resources/car.viv");
    //Load Car data from unpacked NFS files
    std::vector<Model> meshes = nfs_loader.getMeshes();
    meshes[0].enable();
    //Load Track Data
    trk_loader trkLoader("../resources/TRK006/TR06.frd");
    std::map<short, GLuint> gl_id_map = trkLoader.getTextureGLMap();
    std::vector<TrackBlock> track_blocks = trkLoader.getTrackBlocks();
    // TODO: Reference COLs to track blocks
    //std::vector<Model> col_models = trkLoader.getCOLModels();
    //meshes.insert(meshes.end(), col_models.begin(), col_models.end());

    /*------- BULLET --------*/
    btBroadphaseInterface *broadphase = new btDbvtBroadphase();
    // Set up the collision configuration and dispatcher
    btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
    auto *dispatcher = new btCollisionDispatcher(collisionConfiguration);
    // The actual physics solver
    auto *solver = new btSequentialImpulseConstraintSolver;
    // The world.
    auto *dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
    BulletDebugDrawer_DeprecatedOpenGL mydebugdrawer;
    dynamicsWorld->setDebugDrawer(&mydebugdrawer);

    /*------- ImGui -------*/
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    // Setup style
    ImGui::StyleColorsDark();

    // Create and compile our GLSL programs from the shaders
    TrackShader trackShader;
    CarShader carShader;

    // Data used for culling
    Camera mainCamera(glm::vec3(0, 0, 5), 45.0f);
    std::vector<TrackBlock> activeTrackBlocks;
    glm::vec3 oldWorldPosition(0, 0, 0);
    int closestBlockID = 0;

    /*------- MODELS --------*/
    // Gen VBOs, add to Bullet Physics
    for (auto &mesh : meshes) {
        if (!mesh.genBuffers()) {
            return -1;
        }
        dynamicsWorld->addRigidBody(mesh.rigidBody);
    }
    for (auto &track_block : track_blocks) {
        for (auto &track_block_model : track_block.models) {
            if (!track_block_model.genBuffers()) {
                return -1;
            }
            dynamicsWorld->addRigidBody(track_block_model.rigidBody);
        }
    }

    /*------- UI -------*/
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool window_active = true;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        newFrame(window_active);
        // Compute the MVP matrix from keyboard and mouse input
        mainCamera.computeMatricesFromInputs(window_active, ImGui::GetIO());
        glm::mat4 ProjectionMatrix = mainCamera.ProjectionMatrix;
        glm::mat4 ViewMatrix = mainCamera.ViewMatrix;
        glm::vec3 worldPosition = mainCamera.position;
        mydebugdrawer.SetMatrices(ViewMatrix, ProjectionMatrix);

        // If camera moved
        if ((oldWorldPosition.x != worldPosition.x) && (oldWorldPosition.z != worldPosition.z)) {
            float lowestDistanceSqr = FLT_MAX;
            //Primitive Draw distance
            for (auto &track_block : track_blocks) {
                glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-SIMD_PI/2,0,0)));
                glm::vec3 position = orientation*glm::vec3(track_block.trk.ptCentre.x / 10,track_block.trk.ptCentre.y / 10,track_block.trk.ptCentre.z / 10);
                float distanceSqr = glm::length2(glm::distance(worldPosition, glm::vec3(position.x,position.y, position.z)));
                if (distanceSqr < lowestDistanceSqr) {
                    closestBlockID = track_block.block_id;
                    lowestDistanceSqr = distanceSqr;
                }
            }
            int blockDrawDistance = 15;
            int frontBlock =
                    closestBlockID < track_blocks.size() - blockDrawDistance ? closestBlockID + blockDrawDistance
                                                                             : track_blocks.size();
            int backBlock = closestBlockID - blockDrawDistance > 0 ? closestBlockID - blockDrawDistance : 0;
            std::vector<TrackBlock>::const_iterator first = track_blocks.begin() + backBlock;
            std::vector<TrackBlock>::const_iterator last = track_blocks.begin() + frontBlock;
            activeTrackBlocks = std::vector<TrackBlock>(first, last);
            oldWorldPosition = worldPosition;
        }

        // Draw Car
        carShader.use();
        for (auto &mesh : meshes) {
            mesh.update();
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * mesh.ModelMatrix;
            // Send our transformation to the currently bound shader, in the "MVP" uniform
            carShader.loadMVPMatrix(MVP);
            carShader.loadCarColor(glm::vec3(clear_color.x, clear_color.y, clear_color.z));
            carShader.loadCarTexture();
            mesh.render();
        }
        carShader.unbind();

        // Draw TrackBlocks
        trackShader.use();
        for (auto &active_track_Block : activeTrackBlocks) {
            for (auto &track_block_model : active_track_Block.models) {
                track_block_model.update();
                glm::mat4 MVP = ProjectionMatrix * ViewMatrix * track_block_model.ModelMatrix;
                // Send our transformation to the currently bound shader, in the "MVP" uniform
                trackShader.loadMVPMatrix(MVP);
                trackShader.loadSkyColor(glm::vec3(clear_color.x, clear_color.y, clear_color.z));
                trackShader.bindTrackTextures(track_block_model, gl_id_map);
                track_block_model.render();
            }
        }
        trackShader.unbind();

        // Draw UI (Tactically)
        static float f = 0.0f;
        ImGui::Text("NFS3 Engine");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        std::stringstream world_position_string;
        world_position_string << "X " << std::to_string(worldPosition.x) << " Y " << std::to_string(worldPosition.x)
                              << " Z " << std::to_string(worldPosition.z);
        ImGui::Text(world_position_string.str().c_str());
        ImGui::Text(("Block ID: " + std::to_string(closestBlockID)).c_str());
        if (ImGui::Button("Reset View")) {
            mainCamera.resetView();
        };
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("Frag Shader Input", (float *) &clear_color); // Edit 3 floats representing a color
        for (auto &mesh : meshes) {
            ImGui::Checkbox((mesh.getName() + std::to_string(mesh.id)).c_str(), &mesh.enabled);      // Edit bools storing model draw state
        }
        if (ImGui::TreeNode("Track Blocks")) {
            for (auto &track_block : track_blocks) {
                if (ImGui::TreeNode((void *) track_block.block_id, "Track Block %d", track_block.block_id)) {
                    for (auto &block_model : track_block.models) {
                        if (ImGui::TreeNode((void *) block_model.id, "%s %d", block_model.getName().c_str(), block_model.id)) {
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

    // Cleanup VBOs and shaders
    for (auto &mesh : meshes) {
        mesh.destroy();
    }

    carShader.cleanup();
    trackShader.cleanup();
    ImGui_ImplGlfwGL3_Shutdown();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
