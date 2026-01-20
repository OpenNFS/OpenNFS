#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

namespace OpenNFS {
    enum WindowStatus : uint8_t {
        UI,
        GAME
    };

    // Attenuate scroll towards 0 over time
    constexpr double kScrollAttenuation = 0.85;

    class InputManager {
      public:
        bool accelerate{};
        bool reverse{};
        bool brakes{};
        bool handbrakes{};
        bool left{};
        bool right{};
        bool lights{};
        bool shiftUp{};
        bool shiftDown{};
        bool reset{};
        bool escape{};
        bool cameraForwards{};
        bool cameraBackwards{};
        bool cameraLeft{};
        bool cameraRight{};
        double cursorX{};
        double cursorY{};
        double uiCursorX{};
        double uiCursorY{};
        bool mouseLeft{};
        bool mouseRight{};
        bool camSpeedUp{};
        double scrollY{};
        double mouseDeltaX{};
        double mouseDeltaY{};

        explicit InputManager(std::shared_ptr<GLFWwindow> const &window);
        void Scan();
        void ResetCursorPosition() const;
        [[nodiscard]] WindowStatus GetWindowStatus() const;

      private:
        std::shared_ptr<GLFWwindow> m_window;
        WindowStatus m_windowStatus{GAME};
        bool m_lightsKeyPrev{};
        bool m_shiftUpKeyPrev{};
        bool m_shiftDownKeyPrev{};

        static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    };
} // namespace OpenNFS
