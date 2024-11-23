#include "InputManager.h"

namespace OpenNFS {
    InputManager::InputManager(const std::shared_ptr<GLFWwindow> &window) : m_window(window) {

     }

     void InputManager::Scan() {
        glfwGetKey(m_window.get(), GLFW_KEY_W) == GLFW_PRESS,
        glfwGetKey(m_window.get(), GLFW_KEY_S) == GLFW_PRESS;
        glfwGetKey(m_window.get(), GLFW_KEY_SPACE) == GLFW_PRESS;
        glfwGetKey(m_window.get(), GLFW_KEY_D) == GLFW_PRESS;
        glfwGetKey(m_window.get(), GLFW_KEY_A) == GLFW_PRESS;

        if (glfwGetKey(m_window.get(), GLFW_KEY_R) == GLFW_PRESS) {
            ResetToVroad(m_nearestVroadID, 0.f);
        }
        }
} // OpenNFS