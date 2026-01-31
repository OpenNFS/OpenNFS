#pragma once

#include <memory>

#include "../Race/VehicleSelection.h"
#include "GameState.h"
#include "MainMenuState.h"

#include "../UI/UIDropdown.h"
#include "../Menu/CarMenuData.h"

namespace OpenNFS {
    class VehicleSelectionState : public IState {
      public:
        explicit VehicleSelectionState(GameContext &context);

        void OnEnter() override;
        void OnUpdate(float deltaTime) override;
        void OnExit() override;
        GameState GetNextState() const override;

        void LoadCar();
        void ChangeColour();
        void OnGo();

      private:
        GameContext &m_context;
        std::unique_ptr<VehicleSelection> m_vehicleSelection;
        std::shared_ptr<Car> m_currentCar;
        InputManager m_inputManager;
        std::shared_ptr<UIManager> m_uiManager;
        GameState m_nextState{GameState::VehicleSelection};
        std::vector<std::shared_ptr<CarMenuData>> m_cars;
        bool carSelected = false;

        std::shared_ptr<UIDropdown> m_carSelectionDropdown = nullptr;
        std::shared_ptr<UIDropdown> m_colourSelectionDropdown = nullptr;
    };
} // namespace OpenNFS