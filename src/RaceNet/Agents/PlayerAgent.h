#pragma once

#include "CarAgent.h"

#include <GLFW/glfw3.h>

namespace OpenNFS {
    class PlayerAgent final : public CarAgent {
    public:
        PlayerAgent(const std::shared_ptr<GLFWwindow> &window, const std::shared_ptr<Car> &car, const std::shared_ptr<Track> &raceTrack);
        void Simulate() override;

    private:
        std::shared_ptr<GLFWwindow> m_window;
    };
} // namespace OpenNFS
