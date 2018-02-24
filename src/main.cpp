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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureID;
}

// Refactor this into model class render() function
void BindTrackTextures(Model track_block, GLuint TrackTexturesID, std::map<short, GLuint> gl_id_map){
    // TODO: Somehow breaking the CORE profile here?
    GLenum texNum = GL_TEXTURE0;
    for (short texture_id : track_block.texture_ids) {
        glActiveTexture(texNum++);
        glBindTexture( GL_TEXTURE_2D, gl_id_map.find(texture_id)->second);
    }
    const GLint samplers[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    glUniform1iv( TrackTexturesID, 32, samplers );
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

int main(int argc, const char *argv[]) {
    std::cout << "----------- OpenNFS3 v0.01 -----------" << std::endl;
    if (!init_opengl()) {
        std::cout << "OpenGL init failed." << std::endl;
        return -1;
    }

    NFS_Loader nfs_loader("../resources/car.viv");
    if(!nfs_loader.loadObj("../resources/lap3.obj")){
        std::cout << "Obj load failed" << std::endl;
    };
    //Load Car data from unpacked NFS files
    std::vector<Model> meshes = nfs_loader.getMeshes();
    meshes[0].enable();

    //Load Track Data
    trk_loader trkLoader("../resources/TRK006/TR06.frd");
    std::map<short, GLuint> gl_id_map = trkLoader.getTextureGLMap();
    std::vector<Model> track_models = trkLoader.getTrackBlocks();
    meshes.insert(meshes.end(), track_models.begin(), track_models.end());

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

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("../shaders/TransformVertexShader.vertexshader",
                                   "../shaders/TextureFragmentShader.fragmentshader");
    GLuint debugProgramID = LoadShaders("../shaders/TransformVertexShader.vertexshader",
                                   "../shaders/TrackDebugShader.fragmentshader");

    // Load the texture
    GLuint Texture = load_tga_texture("car00.tga");
    // Get handles for uniforms
    GLint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
    GLuint ColorID = glGetUniformLocation(programID, "color");
    GLuint TrackTexturesID = glGetUniformLocation(debugProgramID, "texture_array");

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    /*------- MODELS --------*/
    // Gen VBOs, add to Bullet Physics
    for (auto &mesh : meshes) {
        if (!mesh.genBuffers()) {
            return -1;
        }
        // TODO: Assign shaders to Models in a better way.
        mesh.setShaderID(mesh.track ? debugProgramID : programID);
        dynamicsWorld->addRigidBody(mesh.rigidBody);
    }
    /*------- UI -------*/
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool window_active = true;

    while (!glfwWindowShouldClose(window)) {
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
        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs(window_active, ImGui::GetIO());
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::vec3 worldPosition = getPosition();

        // Draw Meshes
        for (auto &mesh : meshes) {
            // Use our shader
            glUseProgram(mesh.shader_id);
            mesh.update();
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * mesh.ModelMatrix;
            // Send our transformation to the currently bound shader, in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniform3f(ColorID, clear_color.x, clear_color.y, clear_color.z);
            if (mesh.track) {
                BindTrackTextures(mesh, TrackTexturesID, gl_id_map);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, Texture);
            }
            mesh.render();
        }

        // Draw UI (Tactically)
        static float f = 0.0f;
        static int counter = 0;
        ImGui::Text("NFS3 Engine");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        std::stringstream world_position_string;
        world_position_string << "X " << std::to_string(worldPosition.x)  << " Y " << std::to_string(worldPosition.x) << " Z " << std::to_string(worldPosition.z);
        ImGui::Text(world_position_string.str().c_str());
        if (ImGui::Button("Reset View")) {
            resetView();
        };
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("Frag Shader Input", (float *) &clear_color); // Edit 3 floats representing a color
        for (auto &mesh : meshes) {
            ImGui::Checkbox(mesh.getName().c_str(), &mesh.enabled);      // Edit bools storing model draw state
        }

        mydebugdrawer.SetMatrices(ViewMatrix, ProjectionMatrix);
        //dynamicsWorld->debugDrawWorld();

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
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);
    ImGui_ImplGlfwGL3_Shutdown();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


