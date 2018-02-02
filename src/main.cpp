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
#include "TGALoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// Source
#include "shader.h"
#include "controls.h"
#include "nfs_loader.h"

GLFWwindow *window;

std::vector<glm::vec2> loadUVS(const char *path) {
    std::vector<glm::vec2> temp_uvs;

    FILE *obj_file = fopen(path, "r");

    while (1) {
        char lineHeader[128];
        // Read first word from line
        int res = fscanf(obj_file, "%s", lineHeader);
        if (res == EOF) break;

        glm::vec2 uv{};
        fscanf(obj_file, "%f %f\n", &uv.x, &uv.y);
        temp_uvs.push_back(uv);
    }

    fclose(obj_file);

    return temp_uvs;
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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    NFS_Loader nfs_loader("car.viv");
    nfs_loader.writeObj("Model.obj");
    //Load OpenGL data from unpacked NFS files
    std::vector<NFS3_Mesh> meshes = nfs_loader.getMeshes();
    int meshNum = 0;
    std::vector<glm::vec3> car_verts = meshes[meshNum].getVertices();
    std::vector<glm::vec3> car_norms = meshes[meshNum].getNormals();
    std::vector<glm::vec2> car_uvs = meshes[meshNum].getUVs();
    std::vector<unsigned int> car_indices = meshes[meshNum].getIndices();
    //Load Track Data
    std::vector<glm::vec3> track_verts = std::vector<glm::vec3>();
    std::vector<glm::vec3> track_norms = std::vector<glm::vec3>();
    std::vector<glm::vec2> track_uvs = std::vector<glm::vec2>();
    std::vector<unsigned int> track_indices = std::vector<unsigned int>();

    // Read our Track .obj file
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "lap3.obj")) {
        std::cout << err << std::endl;
        return -1;
    }
    // Loop over shapes
    for(size_t s = 0; s < shapes.size(); ++s){
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                track_indices.push_back(idx.vertex_index);
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                track_verts.push_back(glm::vec3(vx * 0.01, vy * 0.01, vz * 0.01));
                tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
                track_norms.push_back(glm::vec3(nx, ny, nz));
                tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                track_uvs.push_back(glm::vec2(tx, ty));
            }
            index_offset += fv;
            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

    if (!init_opengl()) {
        std::cout << "OpenGL init failed." << std::endl;
        return -1;
    }

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("../shaders/TransformVertexShader.vertexshader",
                                   "../shaders/TextureFragmentShader.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLint MatrixID = glGetUniformLocation(programID, "MVP");

    // Load the texture
    GLuint Texture = load_tga_texture("car00.tga");
    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    /*------- CAR --------*/
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    // Load car_verts into a VBO
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, car_verts.size() * sizeof(glm::vec3), &car_verts[0], GL_STATIC_DRAW);
    // UVs
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, car_uvs.size() * sizeof(glm::vec2), &car_uvs[0], GL_STATIC_DRAW);
    // Indices
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, car_indices.size() * sizeof(unsigned int), &car_indices[0], GL_STATIC_DRAW);
    // Normals
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, car_norms.size() * sizeof(glm::vec3), &car_norms[0], GL_STATIC_DRAW);

    /*------- TRACK --------*/
    // Load track into a VBO
    GLuint track_vertexbuffer;
    glGenBuffers(1, &track_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, track_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, track_verts.size() * sizeof(glm::vec3), &track_verts[0], GL_STATIC_DRAW);
    // UVs
    GLuint track_uvbuffer;
    glGenBuffers(1, &track_uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, track_uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, track_uvs.size() * sizeof(glm::vec2), &track_uvs[0], GL_STATIC_DRAW);
    // Indices
    GLuint track_elementbuffer;
    glGenBuffers(1, &track_elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, track_elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, track_indices.size() * sizeof(unsigned int), &track_indices[0],
                 GL_STATIC_DRAW);
    // Normals
    GLuint track_normalbuffer;
    glGenBuffers(1, &track_normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, track_normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, track_norms.size() * sizeof(glm::vec3), &track_norms[0], GL_STATIC_DRAW);

    do {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Use our shader
        glUseProgram(programID);
        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // 1st attribute buffer : car_verts
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void *) 0            // array buffer offset
        );
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
                1,                                // attribute
                2,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                (void *) 0                          // array buffer offset
        );
        // 3rd attribute buffer : Normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
                2,                  // attribute
                3,                  // size
                GL_FLOAT,           // type
                GL_TRUE,           // normalized?
                0,                  // stride
                (void *) 0            // array buffer offset
        );
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        // Draw the triangles !
        /*glDrawElements(
                GL_TRIANGLES,      // mode
                car_indices.size(),    // count
                GL_UNSIGNED_INT,   // type
                (void *) 0           // element array buffer offset
        );*/

        // 1st attribute buffer : track_verts
        glBindBuffer(GL_ARRAY_BUFFER, track_vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void *) 0            // array buffer offset
        );
        // 2nd attribute buffer : UVs
        glBindBuffer(GL_ARRAY_BUFFER, track_uvbuffer);
        glVertexAttribPointer(
                1,                                // attribute
                2,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                (void *) 0                          // array buffer offset
        );
        // 3rd attribute buffer : Normals
        glBindBuffer(GL_ARRAY_BUFFER, track_normalbuffer);
        glVertexAttribPointer(
                2,                  // attribute
                3,                  // size
                GL_FLOAT,           // type
                GL_TRUE,           // normalized?
                0,                  // stride
                (void *) 0            // array buffer offset
        );
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, track_elementbuffer);
        // Draw the triangles !
        glDrawElements(
                GL_TRIANGLES,      // mode
                track_indices.size()-1,    // count
                GL_UNSIGNED_INT,   // type
                (void *) 1           // element array buffer offset
        );
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &track_vertexbuffer);
    glDeleteBuffers(1, &track_uvbuffer);
    glDeleteBuffers(1, &track_normalbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

