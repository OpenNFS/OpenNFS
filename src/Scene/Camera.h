#pragma once

#include "TrackBlock.h"

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "../Renderer/HermiteCurve.h"
#include "../Physics/Car.h"
#include "../nfs_data.h"

class Camera {
private:
    // Initial Field of View
    float initialFoV;
    float speed = 3.0f; // 3 units / second
    float mouseSpeed = 0.005f;
    GLFWwindow* window;
    bool hasSpline = false;

    // NFS Camera Spline
    std::vector<SHARED::CANPT> cameraAnimPoints;
    uint8_t animationPosition = 0;
    float animationDelay = 0.2f; // How much time should elapse until moving to next position in ms
    float animationDeltaTime = 0.f;

    // Cam Spline
    HermiteCurve cameraSpline;

    void calculateZoom();
    void calculatePitch();
    void calculateAngleAroundCar();
    float calculateHorizontalDistance();
    float calculateVerticalDistance();
    void calculateCameraPosition(const std::shared_ptr<Car> &target_car, float horizDistance, float vertDistance);
public:
    Camera(glm::vec3 initial_position, float FoV, float horizontal_angle, float vertical_angle, GLFWwindow *gl_window);
    Camera();

    void resetView();
    void generateSpline(const std::vector<TrackBlock> &trackBlocks);
    void useSpline(float elapsedTime); // Move to position on spline dependent on how long game has been running
    void computeMatricesFromInputs(bool &window_active, float deltaTime);
    void followCar(const std::shared_ptr<Car> &target_car, bool &window_active);
    bool playAnimation(glm::vec3 playerCarPosition);
    void setCameraAnimation(const std::vector<SHARED::CANPT> &canPoints);

    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::vec3 initialPosition;
    // Free look
    // Initial horizontal angle : toward -Z
    float horizontalAngle;
    // Initial vertical angle : none
    float verticalAngle;
    glm::vec3 direction;

    // Cam Spline
    int loopTime;

    // Car Camera
    glm::vec3 position;
    float distanceFromCar = 0.7f;
    float angleAroundCar = 0.f;
    float pitch = 10;
    float yaw = 0;
    float roll = 0;
};

