#include "InputManager.h"

#include "../Config.h"

namespace OpenNFS {
    InputManager::InputManager(std::shared_ptr<GLFWwindow> const &window) : m_window(window) {
        glfwSetWindowUserPointer(m_window.get(), this);
        glfwSetScrollCallback(m_window.get(), ScrollCallback);
    }

    void InputManager::Scan() {
        accelerate = glfwGetKey(m_window.get(), GLFW_KEY_W) == GLFW_PRESS;
        reverse = glfwGetKey(m_window.get(), GLFW_KEY_S) == GLFW_PRESS;
        brakes = glfwGetKey(m_window.get(), GLFW_KEY_SPACE) == GLFW_PRESS;
        right = glfwGetKey(m_window.get(), GLFW_KEY_D) == GLFW_PRESS;
        left = glfwGetKey(m_window.get(), GLFW_KEY_A) == GLFW_PRESS;
        reset = glfwGetKey(m_window.get(), GLFW_KEY_R) == GLFW_PRESS;
        escape = glfwGetKey(m_window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS;
        camSpeedUp = glfwGetKey(m_window.get(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        cameraForwards = accelerate;
        cameraBackwards = reverse;
        cameraLeft = left;
        cameraRight = right;

        // Calculate mouse delta
        double currentX, currentY;
        glfwGetCursorPos(m_window.get(), &currentX, &currentY);
        mouseDeltaX = currentX - cursorX;
        mouseDeltaY = currentY - cursorY;
        cursorX = currentX;
        cursorY = currentY;
        mouseLeft = (glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        mouseRight = (glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        scrollY *= kScrollAttenuation;

        // Cursor Y needs to be inverted + coordinates need normalising
        float const windowToResRatioX{static_cast<float>(Config::get().resX) / static_cast<float>(Config::get().windowSizeX)};
        float const windowToResRatioY{static_cast<float>(Config::get().resY) / static_cast<float>(Config::get().windowSizeY)};
        uiCursorX = cursorX * windowToResRatioX;
        uiCursorY = Config::get().resY - (cursorY * windowToResRatioY);

        // float const windowToResRatioX{(float)Config::get().resX / (float)Config::get().windowSizeX};
        // float const windowToResRatioY{(float)Config::get().resY / (float)Config::get().windowSizeY};
        // cursorX = cursorX * windowToResRatioX;
        // cursorY = Config::get().resY - (cursorY * windowToResRatioY);

        // Detect a click on the 3D Window by detecting a click that isn't on ImGui
        if (mouseLeft && !ImGui::GetIO().WantCaptureMouse) {
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
            inputManager->scrollY = yoffset;
        }
    }
} // namespace OpenNFS
