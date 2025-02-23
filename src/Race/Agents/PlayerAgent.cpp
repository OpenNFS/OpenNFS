#include "PlayerAgent.h"

#include "../../Physics/Car.h"

namespace OpenNFS {
    PlayerAgent::PlayerAgent(const InputManager &inputManager, const std::shared_ptr<Car> &car,
                             const Track &raceTrack) : CarAgent(
                                                                            PLAYER, car, raceTrack), m_inputManager(inputManager) {
        name = "DumbPanda";
    }

    void PlayerAgent::Simulate() {
        // Update data required for efficient track physics update
        this->_UpdateNearestTrackblock();
        this->_UpdateNearestVroad();

        vehicle->ApplyAccelerationForce(m_inputManager.inputs.accelerate, m_inputManager.inputs.reverse);
        vehicle->ApplyBrakingForce(m_inputManager.inputs.brakes);
        vehicle->ApplySteeringRight(m_inputManager.inputs.right);
        vehicle->ApplySteeringLeft(m_inputManager.inputs.left);

        if (m_inputManager.inputs.reset) {
            ResetToVroad(m_nearestVroadID, 0.f);
        }
    }
} // namespace OpenNFS
