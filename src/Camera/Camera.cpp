#include "Camera.h"

Camera::Camera(CameraMode mode, glm::vec3 initialPosition, GLFWwindow *window) {
    m_window = window;
    m_fov = Config::get().fov;
    // Initial position : on +Z
    position = initialPosition;
    m_initialPosition = initialPosition;
    // Projection matrix : 45deg Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    projectionMatrix = glm::perspective(glm::radians(m_fov), 4.0f / 3.0f, 0.1f, 1000.0f);
    m_mode = mode;
}

void Camera::ResetView() {
    position = m_initialPosition;
    m_horizontalAngle = 3.14f;
    m_verticalAngle = 0.0f;
    direction = glm::vec3 (
            cos(m_verticalAngle) * sin(m_horizontalAngle),
            sin(m_verticalAngle),
            cos(m_verticalAngle) * cos(m_horizontalAngle)
    );
    glm::vec3 right = glm::vec3(
            sin(m_horizontalAngle - 3.14f / 2.0f),
            0,
            cos(m_horizontalAngle - 3.14f / 2.0f)
    );
    // Reset view matrix
    viewMatrix = glm::lookAt(position, position + direction, glm::cross(right, direction));
}

void Camera::_UpdateFrustum() {
    viewFrustum.Update(projectionMatrix * viewMatrix);
}

