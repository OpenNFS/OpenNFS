#include "BaseCamera.h"

#include "../Config.h"

namespace OpenNFS {
BaseCamera::BaseCamera(CameraMode const mode, InputManager const &inputManager) : m_inputManager(inputManager) {
    m_fov = Config::get().fov;
    // Initial position : on +Z
    position = glm::vec3(0, 0, 0);
    // Projection matrix : 45deg Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    projectionMatrix = glm::perspective(glm::radians(m_fov), 4.0f / 3.0f, 0.01f, 1000.0f);
    m_mode = mode;
}

void BaseCamera::ResetView() {
    position = glm::vec3(0, 0, 0);
    m_horizontalAngle = 3.14f;
    m_verticalAngle = 0.0f;
    m_direction = glm::vec3(cos(m_verticalAngle) * sin(m_horizontalAngle), sin(m_verticalAngle),
                            cos(m_verticalAngle) * cos(m_horizontalAngle));
    glm::vec3 const right{sin(m_horizontalAngle - 3.14f / 2.0f), 0, cos(m_horizontalAngle - 3.14f / 2.0f)};
    // Reset view matrix
    viewMatrix = glm::lookAt(position, position + m_direction, glm::cross(right, m_direction));
}

void BaseCamera::UpdateFrustum() {
    viewFrustum.Update(projectionMatrix * viewMatrix);
}
} // namespace OpenNFS
