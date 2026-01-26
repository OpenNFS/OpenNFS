#include "FreeCamera.h"

#include "../Config.h"

namespace OpenNFS {
    FreeCamera::FreeCamera(InputManager const &inputManager, glm::vec3 const initialPosition)
        : BaseCamera(CameraMode::FREE_LOOK, inputManager) {
        position = initialPosition;
    }

    void FreeCamera::ComputeMatricesFromInputs(float const deltaTime) {
        // Get mouse position and compute new orientation with it
        m_horizontalAngle += m_mouseSpeedDamper * (Config::get().windowSizeX / 2 - m_inputManager.cursorX);
        m_verticalAngle += m_mouseSpeedDamper * (Config::get().windowSizeY / 2 - m_inputManager.cursorY);

        // Reset mouse position for next frame
        m_inputManager.ResetCursorPosition();

        // Direction : Spherical coordinates to Cartesian coordinates conversion
        m_direction =
            glm::vec3(cos(m_verticalAngle) * sin(m_horizontalAngle), sin(m_verticalAngle), cos(m_verticalAngle) * cos(m_horizontalAngle));

        // Right vector
        glm::vec3 const right{sin(m_horizontalAngle - 3.14f / 2.0f), 0, cos(m_horizontalAngle - 3.14f / 2.0f)};

        // Up vector
        glm::vec3 const up{glm::cross(right, m_direction)};

        // Speed boost
        m_speed = m_inputManager.camSpeedUp ? 200.0f : 30.0f;

        // Move forward
        if (m_inputManager.cameraForwards) {
            position += m_direction * deltaTime * m_speed;
        }
        // Move backward
        if (m_inputManager.cameraBackwards) {
            position -= m_direction * deltaTime * m_speed;
        }
        // Strafe right
        if (m_inputManager.cameraRight) {
            position += right * deltaTime * m_speed;
        }
        // Strafe left
        if (m_inputManager.cameraLeft) {
            position -= right * deltaTime * m_speed;
        }

        // Camera matrix
        viewMatrix = glm::lookAt(position,               // Camera is here
                                 position + m_direction, // and looks here : at the same position, plus "direction"
                                 up                      // Head is up (set to 0,-1,0 to look upside-down)
        );
    }
} // namespace OpenNFS
