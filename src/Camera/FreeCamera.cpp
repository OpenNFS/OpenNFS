#include "FreeCamera.h"

#include "../Config.h"

FreeCamera::FreeCamera(const std::shared_ptr<GLFWwindow> &window, glm::vec3 initialPosition) : BaseCamera(CameraMode::FREE_LOOK, window) {
    position = initialPosition;
}

void FreeCamera::ComputeMatricesFromInputs(float deltaTime) {
    // Get mouse position and compute new orientation with it
    m_horizontalAngle += m_mouseSpeedDamper * (Config::get().resX / 2 - ImGui::GetIO().MousePos.x);
    m_verticalAngle += m_mouseSpeedDamper * (Config::get().resY / 2 - ImGui::GetIO().MousePos.y);

    // Reset mouse position for next frame
    glfwSetCursorPos(m_window.get(), Config::get().resX / 2, Config::get().resY / 2);

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    m_direction = glm::vec3(cos(m_verticalAngle) * sin(m_horizontalAngle), sin(m_verticalAngle), cos(m_verticalAngle) * cos(m_horizontalAngle));

    // Right vector
    glm::vec3 right = glm::vec3(sin(m_horizontalAngle - 3.14f / 2.0f), 0, cos(m_horizontalAngle - 3.14f / 2.0f));

    // Up vector
    glm::vec3 up = glm::cross(right, m_direction);

    // Speed boost
    if (ImGui::GetIO().KeyShift) {
        m_speed = 100.0f;
    } else {
        m_speed = 3.0f;
    }

    if (ImGui::GetIO().MouseDown[1]) {
        // Move forward
        if (glfwGetKey(m_window.get(), GLFW_KEY_W) == GLFW_PRESS) {
            position += m_direction * deltaTime * m_speed;
        }
        // Move backward
        if (glfwGetKey(m_window.get(), GLFW_KEY_S) == GLFW_PRESS) {
            position -= m_direction * deltaTime * m_speed;
        }
        // Strafe right
        if (glfwGetKey(m_window.get(), GLFW_KEY_D) == GLFW_PRESS) {
            position += right * deltaTime * m_speed;
        }
        // Strafe left
        if (glfwGetKey(m_window.get(), GLFW_KEY_A) == GLFW_PRESS) {
            position -= right * deltaTime * m_speed;
        }
    }

    // Camera matrix
    viewMatrix = glm::lookAt(position,               // Camera is here
                             position + m_direction, // and looks here : at the same position, plus "direction"
                             up                      // Head is up (set to 0,-1,0 to look upside-down)
    );
}