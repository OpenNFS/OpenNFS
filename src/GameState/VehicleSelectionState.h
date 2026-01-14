#pragma once

#include <memory>

#include "../Race/VehicleSelection.h"
#include "GameState.h"
#include "MainMenuState.h"

namespace OpenNFS {
    class VehicleSelectionState : public IState {
      public:
        explicit VehicleSelectionState(GameContext &context);

        void OnEnter() override;
        void OnUpdate(float deltaTime) override;
        void OnExit() override;
        GameState GetNextState() const override;

      private:
        GameContext &m_context;
        std::unique_ptr<VehicleSelection> m_vehicleSelection;
        std::shared_ptr<Car> m_currentCar;
        InputManager m_inputManager;
        std::unique_ptr<UIManager> m_uiManager;
        GameState m_nextState{GameState::VehicleSelection};
    };
} // namespace OpenNFS