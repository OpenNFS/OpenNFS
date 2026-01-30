#include "VehicleSelectionState.h"

#include "../Loaders/CarLoader.h"

namespace OpenNFS {
    VehicleSelectionState::VehicleSelectionState(GameContext &context) : m_context(context), m_inputManager(context.window) {
    }

    void VehicleSelectionState::OnEnter() {
        // Setup callbacks for the main menu
        UILayoutLoader::CallbackRegistry callbacks;
        callbacks["onBack"] = [this]() { m_nextState = GameState::MainMenu; };
        callbacks["onGo"] = [this]() { m_nextState = GameState::TrackSelection; };
        callbacks["onVehicleSelectionChange"] = [this]() { LoadCar(); };

        // Create UI manager with vehicle selection layout
        m_uiManager = std::make_unique<UIManager>("../resources/ui/menu/layout/vehicleSelection.json", callbacks);

        for (NfsAssetList assets : m_context.installedNFS) {
            // Only show cars from the selected NFS
            if (assets.tag != m_context.loadedAssets.carTag)
                continue;
            for(std::string car : assets.cars) {
                if (car.empty())
                    continue;
                if (car.starts_with("traffic"))
                    continue;
                if (car == "knoc")
                    continue;
                m_cars.push_back(CarLoader::LoadCarMenuData(m_context.loadedAssets.carTag, car));
            }
        }

        // Load car list into dropdown
        m_dropdown = std::dynamic_pointer_cast<UIDropdown>(m_uiManager.get()->GetElementWithID("vehicleSelectionDropdown"));
        for (auto car : m_cars) {
            m_dropdown->AddEntry(car->carName);
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
        for (auto car : m_cars) {
            if (m_dropdown->text == car->carName)
                m_context.loadedAssets.car = car->id;
        }
        m_currentCar = CarLoader::LoadCar(m_context.loadedAssets.carTag, m_context.loadedAssets.car);

        // Create vehicle selection view
        m_vehicleSelection = std::make_unique<VehicleSelection>(m_context.window, m_context.installedNFS, m_currentCar);
    }
} // namespace OpenNFS