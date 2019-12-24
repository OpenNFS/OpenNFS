#pragma once

#include "TrackBlock.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "../Renderer/HermiteCurve.h"
#include "../Physics/Car.h"
#include "../Physics/Frustum.h"

class Camera {
public:
    Camera(glm::vec3 initialPosition, HermiteCurve trackCenterSpline, GLFWwindow *window);
    Camera() = default;
    void ResetView();
    void UseSpline(float elapsedTime); // Move to position on spline dependent on how long game has been running
    void ComputeMatricesFromInputs(bool &windowActive, float deltaTime);
    void FollowCar(const std::shared_ptr<Car> &targetCar, bool &windowActive);

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 position;
    Frustum viewFrustum;

private:
    void _CalculateZoom();
    void _CalculatePitch();
    void _CalculateAngleAroundCar();
    float _CalculateHorizontalDistance();
    float _CalculateVerticalDistance();
    void _CalculateCameraPosition(const std::shared_ptr<Car> &target_car, float horizDistance, float vertDistance);
    void _UpdateFrustum();

    GLFWwindow* m_window;
    HermiteCurve m_trackCameraRail;
    glm::vec3 m_direction;
    glm::vec3 m_initialPosition;
    float m_fov;
    float m_horizontalAngle = 0.f;  // Initial horizontal angle : toward -Z
    float m_verticalAngle = 0.f;    // Initial vertical angle : none
    float m_distanceFromCar = 0.7f;
    float m_angleAroundCar = 0.f;
    float m_pitch = 10.f;
    float m_yaw = 0.f;
    float m_roll = 0.f;
    int m_loopTime = 0;
    float m_speed = 3.0f;           // 3 units / second
    float m_mouseSpeedDamper = 0.005f;
};

