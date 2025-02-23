#include "InputManager.h"

#include "../Config.h"

namespace OpenNFS {
    InputManager::InputManager(std::shared_ptr<GLFWwindow> const &window) : m_window(window) {
    }

    void InputManager::Scan() {
        glfwPollEvents();
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

        // Detect a click on the 3D Window by detecting a click that isn't on ImGui
        if ((glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) &&
            !ImGui::GetIO().WantCaptureMouse) {
            m_windowStatus = GAME;
            ImGui::GetIO().MouseDrawCursor = false;
        } else if (glfwGetKey(m_window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            m_windowStatus = UI;
            ImGui::GetIO().MouseDrawCursor = true;
        }
    }

    void InputManager::ResetCursorPosition() const {
        glfwSetCursorPos(m_window.get(), Config::get().windowSizeX / 2, Config::get().windowSizeY / 2);
    }

    WindowStatus InputManager::GetWindowStatus() const {
        return m_windowStatus;
    }
} // namespace OpenNFS
