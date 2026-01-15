#include "PlayerAgent.h"

#include "../../Physics/Car.h"

namespace OpenNFS {
    PlayerAgent::PlayerAgent(InputManager const &inputManager, std::shared_ptr<Car> const &car, std::shared_ptr<Track> const &raceTrack)
        : CarAgent(PLAYER, car, raceTrack), m_inputManager(inputManager) {
        name = "DumbPanda";
    }

    void PlayerAgent::Simulate() {
        // Update data required for efficient track physics update
        this->_UpdateNearestTrackblock();
        this->_UpdateNearestVroad();

        vehicle->ApplyAccelerationForce(m_inputManager.accelerate, m_inputManager.reverse);
        vehicle->ApplyBrakingForce(m_inputManager.brakes);
        vehicle->ApplySteeringRight(m_inputManager.right);
        vehicle->ApplySteeringLeft(m_inputManager.left);

        if (m_inputManager.reset) {
            ResetToVroad(m_nearestVroadID, 0.f);
        }

        if (m_inputManager.lights) {
            vehicle->ToggleLights();
        }
    }
} // namespace OpenNFS
