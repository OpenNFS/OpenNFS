#include "VehicleSelectionState.h"

#include "../Loaders/CarLoader.h"

namespace OpenNFS {
    VehicleSelectionState::VehicleSelectionState(GameContext &context) : m_context(context), m_inputManager(context.window) {
    }

    void VehicleSelectionState::OnEnter() {
        // Setup callbacks for the main menu
        UILayoutLoader::CallbackRegistry callbacks;
        callbacks["onBack"] = [this]() { m_nextState = GameState::MainMenu; };
        callbacks["onGo"] = [this]() { OnGo(); };
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
        // Sort the cars alphabetically
        auto compareFunc  = [](std::shared_ptr<CarMenuData> a, std::shared_ptr<CarMenuData> b) {return a->carName<b->carName;};
        std::sort(m_cars.begin(), m_cars.end(), compareFunc);

        // Load car list into dropdown
        m_carSelectionDropdown = std::dynamic_pointer_cast<UIDropdown>(m_uiManager.get()->GetElementWithID("vehicleSelectionDropdown"));
        m_colourSelectionDropdown = std::dynamic_pointer_cast<UIDropdown>(m_uiManager.get()->GetElementWithID("vehicleColourSelectionDropdown"));
        for (auto car : m_cars) {
            m_carSelectionDropdown->AddEntry(car->carName);
        }

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

        if (carSelected && m_vehicleSelection) {
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
        size_t selection = m_carSelectionDropdown->GetSelectedEntryIndex();
        if (selection == -1) {
            return;
        }
        m_context.loadedAssets.car = m_cars[selection]->id;
        m_currentCar = CarLoader::LoadCar(m_context.loadedAssets.carTag, m_context.loadedAssets.car);

        // Create vehicle selection view
        m_vehicleSelection = std::make_unique<VehicleSelection>(m_context.window, m_context.installedNFS, m_currentCar);

        // Set car colours
        if (!m_currentCar->assetData.metadata.colours.empty()) {
            m_colourSelectionDropdown->entries.clear();
            for (size_t i = 0; i< m_currentCar->assetData.metadata.colours.size(); i++) {
                m_colourSelectionDropdown->AddEntry(m_cars[selection]->colorNames[i]);
                if (m_currentCar->assetData.metadata.colours[i].colour == m_currentCar->vehicleState.colour) {
                    m_colourSelectionDropdown->selectedEntry = i;
                    m_colourSelectionDropdown->text = m_cars[selection]->colorNames[i];
                    m_context.loadedAssets.colour = i;
                }
            }
            m_colourSelectionDropdown->isVisible = true;

        } else {
            m_colourSelectionDropdown->isVisible = false;
        }
        carSelected = true;
    }

    void VehicleSelectionState::OnGo() {
        if (carSelected)
            m_nextState = GameState::TrackSelection;
    }
} // namespace OpenNFS