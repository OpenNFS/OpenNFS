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
public:
    Camera(glm::vec3 initial_position, float FoV, float horizontal_angle, float vertical_angle, GLFWwindow *gl_window);
    Camera();

    void resetView();
    void generateSpline(std::vector<TrackBlock> trackBlocks);
    void useSpline();
    void computeMatricesFromInputs(bool &window_active, ImGuiIO& io);
    void followCar(const shared_ptr<Car> &target_car, bool &window_active, ImGuiIO &io);
    bool playAnimation();
    void setCameraAnimation(std::vector<NFS3_4_DATA::ANIMDATA> canPoints);

    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::vec3 initialPosition;
    float deltaTime;

    // Car Camera
    glm::vec3 position;
    float distanceFromCar = 0.7f;
    float angleAroundCar = 0;
    float pitch = 10;
    float yaw = 0;
    float roll = 0;

    // Free look
    // Initial horizontal angle : toward -Z
    float horizontalAngle;
    // Initial vertical angle : none
    float verticalAngle;

    // NFS Camera Spline
    std::vector<NFS3_4_DATA::ANIMDATA> cameraAnimPoints;
    uint8_t animationPosition = 0;
    float animationDelay = 0.2f; // How much time should elapse until moving to next position in ms
    float animationDeltaTime = 0.f;

    // Cam Spline
    HermiteCurve cameraSpline;
    float totalTime = 1;
    int loopTime;

    void calculateZoom();
    void calculatePitch();
    void calculateAngleAroundCar();
    float calculateHorizontalDistance();
    float calculateVerticalDistance();
    void calculateCameraPosition(const shared_ptr<Car> &target_car, float horizDistance, float vertDistance);
};

