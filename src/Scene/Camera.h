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
    // Initial Field of View
    float initialFoV;
    float speed = 3.0f; // 3 units / second
    float mouseSpeed = 0.005f;

public:
    Camera(glm::vec3 initial_position, float FoV, float horizontal_angle, float vertical_angle);
    void resetView();
    void computeMatricesFromInputs(bool &window_active, ImGuiIO& io);
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::vec3 position;
    // Initial horizontal angle : toward -Z
    float horizontalAngle;
    // Initial vertical angle : none
    float verticalAngle;
    float deltaTime;
};

