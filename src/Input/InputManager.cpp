#include "InputManager.h"

#include "../Config.h"

namespace OpenNFS {
    InputManager::InputManager(const std::shared_ptr<GLFWwindow> &window) : m_window(window) {
    }

    void InputManager::Scan() {
        inputs.accelerate = glfwGetKey(m_window.get(), GLFW_KEY_W) == GLFW_PRESS;
        inputs.reverse = glfwGetKey(m_window.get(), GLFW_KEY_S) == GLFW_PRESS;
        inputs.brakes = glfwGetKey(m_window.get(), GLFW_KEY_SPACE) == GLFW_PRESS;
        inputs.right = glfwGetKey(m_window.get(), GLFW_KEY_D) == GLFW_PRESS;
        inputs.left = glfwGetKey(m_window.get(), GLFW_KEY_A) == GLFW_PRESS;
        inputs.reset = glfwGetKey(m_window.get(), GLFW_KEY_R) == GLFW_PRESS;

        inputs.cameraForwards = inputs.accelerate;
        inputs.cameraBackwards = inputs.reverse;
        inputs.cameraLeft = inputs.left;
        inputs.cameraRight = inputs.right;
    }

    void InputManager::ResetCursorPosition() const {
        glfwSetCursorPos(m_window.get(), Config::get().windowSizeX / 2, Config::get().windowSizeY / 2);
    }
} // OpenNFS
