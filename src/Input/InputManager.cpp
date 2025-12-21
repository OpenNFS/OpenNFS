#include "InputManager.h"

#include "../Config.h"

namespace OpenNFS {
    InputManager::InputManager(std::shared_ptr<GLFWwindow> const &window) : m_window(window) {
        glfwSetWindowUserPointer(m_window.get(), this);
        glfwSetScrollCallback(m_window.get(), ScrollCallback);
    }

    void InputManager::Scan() {
        glfwPollEvents();

        inputs.accelerate = glfwGetKey(m_window.get(), GLFW_KEY_W) == GLFW_PRESS;
        inputs.reverse = glfwGetKey(m_window.get(), GLFW_KEY_S) == GLFW_PRESS;
        inputs.brakes = glfwGetKey(m_window.get(), GLFW_KEY_SPACE) == GLFW_PRESS;
        inputs.right = glfwGetKey(m_window.get(), GLFW_KEY_D) == GLFW_PRESS;
        inputs.left = glfwGetKey(m_window.get(), GLFW_KEY_A) == GLFW_PRESS;
        inputs.reset = glfwGetKey(m_window.get(), GLFW_KEY_R) == GLFW_PRESS;
        inputs.camSpeedUp = glfwGetKey(m_window.get(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        inputs.cameraForwards = inputs.accelerate;
        inputs.cameraBackwards = inputs.reverse;
        inputs.cameraLeft = inputs.left;
        inputs.cameraRight = inputs.right;

        // Calculate mouse delta
        double currentX, currentY;
        glfwGetCursorPos(m_window.get(), &currentX, &currentY);
        inputs.mouseDeltaX = currentX - inputs.cursorX;
        inputs.mouseDeltaY = currentY - inputs.cursorY;
        inputs.cursorX = currentX;
        inputs.cursorY = currentY;
        inputs.mouseLeft = (glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        inputs.mouseRight = (glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        inputs.scrollY *= kScrollAttenuation;

        //float const windowToResRatioX{(float)Config::get().resX / (float)Config::get().windowSizeX};
        //float const windowToResRatioY{(float)Config::get().resY / (float)Config::get().windowSizeY};
        //inputs.cursorX = inputs.cursorX * windowToResRatioX;
        //inputs.cursorY = Config::get().resY - (inputs.cursorY * windowToResRatioY);

        // Detect a click on the 3D Window by detecting a click that isn't on ImGui
        if (inputs.mouseLeft && !ImGui::GetIO().WantCaptureMouse) {
            m_windowStatus = GAME;
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        } else if (glfwGetKey(m_window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            m_windowStatus = UI;
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    void InputManager::ResetCursorPosition() const {
        glfwSetCursorPos(m_window.get(), Config::get().windowSizeX / 2, Config::get().windowSizeY / 2);
    }

    WindowStatus InputManager::GetWindowStatus() const {
        return m_windowStatus;
    }

    void InputManager::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
        auto *inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));
        if (inputManager) {
            inputManager->inputs.scrollY = yoffset;
        }
    }
} // namespace OpenNFS
