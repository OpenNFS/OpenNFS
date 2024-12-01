#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Physics/Frustum.h"
#include "../Input/InputManager.h"

namespace OpenNFS {
    enum CameraMode : uint8_t { FOLLOW_CAR, HERMITE_FLYTHROUGH, FREE_LOOK };

    class BaseCamera {
    public:
        BaseCamera(CameraMode mode, const InputManager &inputManager);

        void UpdateFrustum();

        void ResetView();

        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::vec3 position;
        Frustum viewFrustum;

    protected:
        const InputManager &m_inputManager;
        CameraMode m_mode;
        glm::vec3 m_direction;
        float m_fov;
        float m_horizontalAngle = 0.f; // Initial horizontal angle : toward -Z
        float m_verticalAngle = 0.f; // Initial vertical angle : none
        float m_roll = 0.f;
        float m_speed = 3.0f; // 3 units / second
        float m_mouseSpeedDamper = 0.005f;
    };
}
