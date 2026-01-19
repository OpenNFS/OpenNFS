#include "VehicleSelectionState.h"

#include "../Loaders/CarLoader.h"

namespace OpenNFS {
    VehicleSelectionState::VehicleSelectionState(GameContext &context) : m_context(context), m_inputManager(context.window) {
    }

    void VehicleSelectionState::OnEnter() {
        // Setup callbacks for the main menu
        UILayoutLoader::CallbackRegistry callbacks;
        callbacks["onMainMenu"] = [this]() { m_nextState = GameState::MainMenu; };
        callbacks["onStartRace"] = [this]() { m_nextState = GameState::Race; };
        callbacks["onVehicleSelectionChange"] = [this]() { LoadCar(); };

        // Create UI manager with vehicle selection layout
        m_uiManager = std::make_unique<UIManager>("../resources/ui/menu/layout/vehicleSelection.json", callbacks);

        // Load car list into 
        for (auto element : m_uiManager.get()->m_uiElements) {
            if (element.get()->type == UIElementType::Dropdown) {
                dropdown = (UIDropdown *) element.get();
                break;
            }
        }
        for (NfsAssetList assets : m_context.installedNFS) {
            // Only show cars from the selected NFS
            if (assets.tag != m_context.loadedAssets.carTag)
                continue;
            for(std::string car : assets.cars) {
                if (car.starts_with("traffic"))
                    continue;
                if (car == "cartool.zip")
                    continue;
                if (car == "knoc")
                    continue;
                if (dropdown->entries.size() == 0)
                    dropdown->text = car;
                dropdown->entries.push_back(car);
                dropdown->entryTextColour.push_back(dropdown->textColour);
            }
        }
        LoadCar();
        
        m_vehicleSelection = std::make_unique<VehicleSelection>(m_context.window, m_context.installedNFS, m_currentCar);

        // Reset next state
        m_nextState = GameState::VehicleSelection;
    }

    void VehicleSelectionState::OnUpdate(float const deltaTime) {
        // Update input
        m_inputManager.Scan();

        // Handle escape key to exit
        if (m_inputManager.escape) {
            m_nextState = GameState::MainMenu;
        }

        if (m_vehicleSelection) {
            m_vehicleSelection->Update(deltaTime);

            // Check if race should end
            if (m_vehicleSelection->IsFinished()) {
                LOG(INFO) << "Car selection finished, returning to main menu";
                m_nextState = GameState::MainMenu;
            }
        }

        // Update and render UI
        m_uiManager->Update(m_inputManager);
    }

    void VehicleSelectionState::OnExit() {
        m_vehicleSelection.reset();
    }

    GameState VehicleSelectionState::GetNextState() const {
        return m_nextState;
    }
    void VehicleSelectionState::LoadCar() {
        LOG(INFO) << "Car selection happened";
        m_currentCar = CarLoader::LoadCar(m_context.loadedAssets.carTag, dropdown->text);

        // Create vehicle selection view
        m_vehicleSelection = std::make_unique<VehicleSelection>(m_context.window, m_context.installedNFS, m_currentCar);
    }
} // namespace OpenNFS