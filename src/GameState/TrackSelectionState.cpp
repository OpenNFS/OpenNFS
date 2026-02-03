#include "TrackSelectionState.h"

#include "../Loaders/MenuTextLoader.h"
#include "../Menu/MenuText.h"

namespace OpenNFS {
    TrackSelectionState::TrackSelectionState(GameContext &context) : m_context(context), m_inputManager(context.window) {
    }

    void TrackSelectionState::OnEnter() {
        // Setup callbacks for the main menu
        UILayoutLoader::CallbackRegistry callbacks;
        callbacks["onBack"] = [this]() { m_nextState = GameState::VehicleSelection; };
        callbacks["onGo"] = [this]() { OnGo(); };
        callbacks["onTrackSelectionChange"] = [this]() { LoadTrack(); };
        callbacks["onNFSSelectionChange"] = [this]() { SwitchNFSVersion(); };

        // Create UI manager with vehicle selection layout
        m_uiManager = std::make_unique<UIManager>("../resources/ui/menu/layout/trackSelection.json", callbacks);

        // Load track list into dropdown
        m_trackSelectionDropdown = std::dynamic_pointer_cast<UIDropdown>(m_uiManager.get()->GetElementWithID("trackSelectionDropdown"));
        m_NFSSelectionDropdown = std::dynamic_pointer_cast<UIDropdown>(m_uiManager.get()->GetElementWithID("NFSSelectionDropdown"));
        for (NfsAssetList assets : m_context.installedNFS) {
            std::string const nfsVersion = std::string(magic_enum::enum_name<NFSVersion>(assets.tag));
            m_NFSSelectionDropdown->AddEntry(nfsVersion);
            // Only show tracks from the selected NFS
            if (assets.tag == m_context.loadedAssets.trackTag) {
                m_NFSSelectionDropdown->selectedEntry = m_NFSSelectionDropdown->entries.size() - 1;
            }
        }
        SwitchNFSVersion();
    
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
        int selection = m_trackSelectionDropdown->GetSelectedEntryIndex();
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

    void TrackSelectionState::SwitchNFSVersion() {
        trackSelected = false;
        m_trackSelectionDropdown->selectedEntry = -1;
        m_trackSelectionDropdown->entries.clear();
        m_trackNames.clear();
        m_tracks.clear();

        for (NfsAssetList assets : m_context.installedNFS) {
            if (assets.tag == magic_enum::enum_cast<NFSVersion>(m_NFSSelectionDropdown->entries[m_NFSSelectionDropdown->selectedEntry])) {
                m_context.loadedAssets.trackTag = assets.tag;
                for(std::string track : assets.tracks) {
                    m_tracks.push_back(track);
                }
            }
        }
        // Sort the track alphabetically
        auto compareFunc  = [](std::string a, std::string b) {return a<b;};
        std::sort(m_tracks.begin(), m_tracks.end(), compareFunc);

        // Load track names
        auto const menuText = MenuTextLoader::LoadMenuText(m_context.loadedAssets.trackTag);
        if (menuText->trackNames.empty()) {
            m_trackNames = m_tracks;
        } else {
            for (std::string track : menuText->trackNames) {
                m_trackNames.push_back(track);
            }
        }

        // Add entries to track selection dropdown
        for (auto track : m_trackNames) {
            m_trackSelectionDropdown->AddEntry(track);
        }
    }
} // namespace OpenNFS