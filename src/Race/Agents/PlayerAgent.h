#pragma once

#include "CarAgent.h"
#include "../../Input/InputManager.h"

namespace OpenNFS {
    class PlayerAgent final : public CarAgent {
    public:
        PlayerAgent(const InputManager &inputManager, const std::shared_ptr<Car> &car, const Track &raceTrack);
        void Simulate() override;

    private:
        const InputManager &m_inputManager;
    };
} // namespace OpenNFS
