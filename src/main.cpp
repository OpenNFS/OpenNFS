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
#include <imgui/imgui.h>
#include <btBulletDynamicsCommon.h>
#include <imgui/imgui_impl_glfw_gl3.h>
#include "TGALoader.h"

#define TINYOBJLOADER_IMPLEMENTATION

// Source
#include "shader.h"
#include "controls.h"
#include "nfs_loader.h"
#include "trk_loader.h"

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

std::vector<glm::vec2> loadUVS(const char *path) {
    std::vector<glm::vec2> temp_uvs;

    FILE *obj_file = fopen(path, "r");

    while (1) {
        glm::vec2 uv{};
        int res = fscanf(obj_file, "%f %f\n", &uv.x, &uv.y);
        if (res == EOF) break;
        temp_uvs.push_back(uv);
    }

    fclose(obj_file);

    return temp_uvs;
}

std::vector<glm::vec3> loadVerts(const char *path) {
    std::vector<glm::vec3> temp_verts;

    FILE *obj_file = fopen(path, "r");

    while (1) {
        glm::vec3 vert{};
        int res = fscanf(obj_file, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
        if (res == EOF) break;
        temp_verts.push_back(vert);
    }

    fclose(obj_file);

    return temp_verts;
}

GLint load_tga_texture(const char *path) {
    NS_TGALOADER::IMAGE texture_loader;

    if (!texture_loader.LoadTGA(path)) {
        printf("Texture loading failed!\n");
        exit(2);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // Vintage look (1 - 1 map, no interp)
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureID;
}

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
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "Need for Speed 3 FCE Loader", nullptr, nullptr);

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
    // Hide the mouse and enable unlimited movement
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(10);
    return true;
}

int main(int argc, const char *argv[]) {
    std::cout << "----------- NFS3 Model Viewer v0.5 -----------" << std::endl;
    NFS_Loader nfs_loader("../resources/car_f1.viv");
    if(!nfs_loader.loadObj("../resources/lap3.obj")){
        std::cout << "Track load failed" << std::endl;
    };
    //Load OpenGL data from unpacked NFS files
    std::vector<Model> meshes = nfs_loader.getMeshes();
    meshes[0].enable();

    trk_loader trkLoader("../resources/TR00.frd");
    for(auto &mesh : trkLoader.trk_blocks){
        meshes.push_back(mesh);
    }

    if (!init_opengl()) {
        std::cout << "OpenGL init failed." << std::endl;
        return -1;
    }

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
    ImGui_ImplGlfwGL3_Init(window, true);
    // Setup style
    ImGui::StyleColorsDark();

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("../shaders/TransformVertexShader.vertexshader",
                                   "../shaders/TextureFragmentShader.fragmentshader");
    // Get a handle for our "MVP" uniform
    GLint MatrixID = glGetUniformLocation(programID, "MVP");
    // Load the texture
    GLuint Texture = load_tga_texture("car00.tga");
    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
    GLuint ColorID = glGetUniformLocation(programID, "color");
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    /*------- MODELS --------*/
    // Gen VBOs, add to Bullet Physics
    for (auto &mesh : meshes) {
        if (!mesh.genBuffers()) {
            return -1;
        }
        dynamicsWorld->addRigidBody(mesh.rigidBody);
    }
    /*------- UI -------*/
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool window_active = true;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Use our shader
        glUseProgram(programID);
        // Detect a click on the 3D Window by detecting a click that isn't on ImGui
        window_active = window_active ? window_active : (
                (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) &&
                (!ImGui::GetIO().WantCaptureMouse));
        if (!window_active) {
            ImGui::GetIO().MouseDrawCursor = false;
        }
        ImGui_ImplGlfwGL3_NewFrame();
        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs(window_active, ImGui::GetIO());
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        // Draw Meshes
        for (auto &mesh : meshes) {
            mesh.update();
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * mesh.ModelMatrix;
            // Send our transformation to the currently bound shader, in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniform3f(ColorID, clear_color.x, clear_color.y, clear_color.z);
            mesh.render();
        }

        // Draw UI (Tactically)
        static float f = 0.0f;
        static int counter = 0;
        ImGui::Text("NFS3 Engine");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        if (ImGui::Button("Reset View")) {
            resetView();
        };
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("Frag Shader Input", (float *) &clear_color); // Edit 3 floats representing a color
        for (auto &mesh : meshes) {
            ImGui::Checkbox(mesh.getName().c_str(), &mesh.enabled);      // Edit bools storing model draw state
        }


        ImGui::Text("car00.tga");
        ImGui::Image((ImTextureID) TextureID, ImVec2(256, 256));

        if (show_demo_window) {
            // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
            ImGui::SetNextWindowPos(ImVec2(650, 20),
                                    ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        mydebugdrawer.SetMatrices(ViewMatrix, ProjectionMatrix);
        dynamicsWorld->debugDrawWorld();

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // Cleanup VBOs and shaders
    for (auto &mesh : meshes) {
        mesh.destroy();
    }
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);
    ImGui_ImplGlfwGL3_Shutdown();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


