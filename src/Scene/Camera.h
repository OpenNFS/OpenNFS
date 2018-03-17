#pragma once
// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

extern GLFWwindow* window;

class Camera {
private:
    // Initial horizontal angle : toward -Z
    float horizontalAngle = 3.14f;
    // Initial vertical angle : none
    float verticalAngle = 0.0f;
    // Initial Field of View
    float initialFoV;
    float speed = 3.0f; // 3 units / second
    float mouseSpeed = 0.005f;

public:
    Camera(glm::vec3 initial_position, float FoV);
    void resetView();
    void computeMatricesFromInputs(bool &window_active, ImGuiIO& io);
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::vec3 position;
    float deltaTime;
};

