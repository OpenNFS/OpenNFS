#pragma once

#include "Models/TrackBlock.h"
#include "../Physics/Frustum.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

enum WindowStatus : uint8_t { UI, GAME };

enum CameraMode : uint8_t { FOLLOW_CAR, HERMITE_FLYTHROUGH, FREE_LOOK };

class BaseCamera {
public:
    BaseCamera(CameraMode mode, const std::shared_ptr<GLFWwindow> &window);
    BaseCamera() = default;
    void UpdateFrustum();
    void ResetView();

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 position;
    Frustum viewFrustum;

protected:
    std::shared_ptr<GLFWwindow> m_window;
    CameraMode m_mode;
    glm::vec3 m_direction;
    float m_fov;
    float m_horizontalAngle  = 0.f; // Initial horizontal angle : toward -Z
    float m_verticalAngle    = 0.f; // Initial vertical angle : none
    float m_roll             = 0.f;
    float m_speed            = 3.0f; // 3 units / second
    float m_mouseSpeedDamper = 0.005f;
};
