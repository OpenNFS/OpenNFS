#include "PlayerAgent.h"

PlayerAgent::PlayerAgent(GLFWwindow *pWindow, std::shared_ptr<Car> car, std::shared_ptr<ONFSTrack> raceTrack) :
CarAgent(AgentType::PLAYER, car, raceTrack)
{
    m_pWindow = pWindow;
}

void PlayerAgent::Simulate()
{
    // Update data required for efficient track physics update
    this->_UpdateNearestTrackblock();
    this->_UpdateNearestVroad();

    //if (userParams.windowActive && !ImGui::GetIO().MouseDown[1]) {
    vehicle->ApplyAccelerationForce(glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS, glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS);
    vehicle->ApplyBrakingForce(glfwGetKey(m_pWindow, GLFW_KEY_SPACE) == GLFW_PRESS);
    vehicle->ApplySteeringRight(glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS);
    vehicle->ApplySteeringLeft(glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS);

    if(glfwGetKey(m_pWindow, GLFW_KEY_R) == GLFW_PRESS)
    {
        ResetToVroad(m_nearestVroadID, 0.f);
    }
    //}
}