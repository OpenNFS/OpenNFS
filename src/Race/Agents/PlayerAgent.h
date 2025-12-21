#pragma once

#include "../../Input/InputManager.h"
#include "CarAgent.h"

namespace OpenNFS {
    class PlayerAgent final : public CarAgent {
      public:
        PlayerAgent(InputManager const &inputManager, std::shared_ptr<Car> const &car, Track const &raceTrack);
        void Simulate() override;

      private:
        InputManager const &m_inputManager;
    };
} // namespace OpenNFS
