#include "TrackSelectionState.h"

namespace OpenNFS {
    TrackSelectionState::TrackSelectionState(GameContext &context) : m_context(context), m_inputManager(context.window) {
    }

    void TrackSelectionState::OnEnter() {
        // Setup callbacks for the main menu
        UILayoutLoader::CallbackRegistry callbacks;
        callbacks["onBack"] = [this]() { m_nextState = GameState::VehicleSelection; };
        callbacks["onGo"] = [this]() { OnGo(); };
        callbacks["onTrackSelectionChange"] = [this]() { LoadTrack(); };

        // Create UI manager with vehicle selection layout
        m_uiManager = std::make_unique<UIManager>("../resources/ui/menu/layout/trackSelection.json", callbacks);

        for (NfsAssetList assets : m_context.installedNFS) {
            if (assets.tag != m_context.loadedAssets.trackTag)
                continue;
            for(std::string track : assets.tracks) {
                m_tracks.push_back(track);
            }
        }

        // Sort the cars alphabetically
        auto compareFunc  = [](std::string a, std::string b) {return a<b;};
        std::sort(m_tracks.begin(), m_tracks.end(), compareFunc);

        // Load track list into dropdown
        m_dropdown = std::dynamic_pointer_cast<UIDropdown>(m_uiManager.get()->GetElementWithID("trackSelectionDropdown"));
        for (auto track : m_tracks) {
            m_dropdown->AddEntry(track);
        }

        LoadTrack();
    
        // Reset next state
        m_nextState = GameState::TrackSelection;
    }

    void TrackSelectionState::OnUpdate(float const deltaTime) {
        // Update input
        m_inputManager.Scan();

        // Handle escape key to exit
        if (m_inputManager.escape) {
            m_nextState = GameState::MainMenu;
        }

        // Update and render UI
        m_uiManager->Update(m_inputManager);
    }

    void TrackSelectionState::OnExit() {
    }

    GameState TrackSelectionState::GetNextState() const {
        return m_nextState;
    }

    void TrackSelectionState::LoadTrack() {
        int selection = m_dropdown->GetSelectedEntryIndex();
        if (selection == -1) {
            return;
        }
        m_context.loadedAssets.track = m_tracks[selection];
        trackSelected = true;
    }

    void TrackSelectionState::OnGo() {
        if (trackSelected)
            m_nextState = GameState::RaceLoad;
    }
} // namespace OpenNFS