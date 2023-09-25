#include "PlayerAgent.h"

namespace OpenNFS {
    PlayerAgent::PlayerAgent(const std::shared_ptr<GLFWwindow> &window, const std::shared_ptr<Car> &car, const std::shared_ptr<Track> &raceTrack) :
        CarAgent(AgentType::PLAYER, car, raceTrack), m_window(window) {
        name = "DumbPanda";
    }

    void PlayerAgent::Simulate() {
        // Update data required for efficient track physics update
        this->_UpdateNearestTrackblock();
        this->_UpdateNearestVroad();

        // if (userParams.windowActive && !ImGui::GetIO().MouseDown[1]) { }
        vehicle->ApplyAccelerationForce(glfwGetKey(m_window.get(), GLFW_KEY_W) == GLFW_PRESS, glfwGetKey(m_window.get(), GLFW_KEY_S) == GLFW_PRESS);
        vehicle->ApplyBrakingForce(glfwGetKey(m_window.get(), GLFW_KEY_SPACE) == GLFW_PRESS);
        vehicle->ApplySteeringRight(glfwGetKey(m_window.get(), GLFW_KEY_D) == GLFW_PRESS);
        vehicle->ApplySteeringLeft(glfwGetKey(m_window.get(), GLFW_KEY_A) == GLFW_PRESS);

        if (glfwGetKey(m_window.get(), GLFW_KEY_R) == GLFW_PRESS) {
            ResetToVroad(m_nearestVroadID, 0.f);
        }
    }
} // namespace OpenNFS
