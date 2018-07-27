#pragma once

#include "TrackBlock.h"

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "../Renderer/HermiteCurve.h"


class Camera {
private:
    // Initial Field of View
    float initialFoV;
    float speed = 3.0f; // 3 units / second
    float mouseSpeed = 0.005f;
    GLFWwindow* window;
    bool hasSpline = false;
public:
    Camera(glm::vec3 initial_position, float FoV, float horizontal_angle, float vertical_angle, GLFWwindow *gl_window);
    Camera();

    void resetView();
    void generateSpline(std::vector<TrackBlock> trackBlocks);
    void useSpline();
    void computeMatricesFromInputs(bool &window_active, ImGuiIO& io);
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::vec3 position;
    glm::vec3 initialPosition;
    // Initial horizontal angle : toward -Z
    float horizontalAngle;
    // Initial vertical angle : none
    float verticalAngle;
    float deltaTime;
    // Cam Spline
    HermiteCurve cameraSpline;
    float totalTime = 1;
    int loopTime;
};

