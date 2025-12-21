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
        struct Inputs {
            bool accelerate{};
            bool reverse{};
            bool brakes{};
            bool left{};
            bool right{};
            bool reset{};
            bool cameraForwards{};
            bool cameraBackwards{};
            bool cameraLeft{};
            bool cameraRight{};
            double cursorX{};
            double cursorY{};
            bool mouseLeft{};
            bool mouseRight{};
            bool camSpeedUp{};
            double scrollY{};
            double mouseDeltaX{};
            double mouseDeltaY{};
        };

        explicit InputManager(std::shared_ptr<GLFWwindow> const &window);
        void Scan();
        void ResetCursorPosition() const;
        [[nodiscard]] WindowStatus GetWindowStatus() const;

        Inputs inputs{};

      private:
        std::shared_ptr<GLFWwindow> m_window;
        WindowStatus m_windowStatus{GAME};

        static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    };
} // namespace OpenNFS
