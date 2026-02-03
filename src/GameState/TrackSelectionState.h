#pragma once

#include <memory>

#include "GameState.h"
#include "MainMenuState.h"

#include "../UI/UIDropdown.h"

namespace OpenNFS {
    class TrackSelectionState : public IState {
      public:
        explicit TrackSelectionState(GameContext &context);

        void OnEnter() override;
        void OnUpdate(float deltaTime) override;
        void OnExit() override;
        GameState GetNextState() const override;

        void LoadTrack();
        void OnGo();
        void SwitchNFSVersion();

      private:
        GameContext &m_context;
        InputManager m_inputManager;
        std::shared_ptr<UIManager> m_uiManager;
        GameState m_nextState{GameState::TrackSelection};
        std::vector<std::string> m_tracks;
        std::vector<std::string> m_trackNames;
        bool trackSelected = false;

        std::shared_ptr<UIDropdown> m_trackSelectionDropdown = nullptr;
        std::shared_ptr<UIDropdown> m_NFSSelectionDropdown = nullptr;
    };
} // namespace OpenNFS