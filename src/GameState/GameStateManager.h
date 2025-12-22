#pragma once

#include <memory>
#include <unordered_map>

#include "GameState.h"

namespace OpenNFS {
    class GameStateManager {
      public:
        GameStateManager();

        // Register a state with the manager
        void RegisterState(GameState stateType, std::unique_ptr<IState> state);

        // Transition to a new state
        void TransitionTo(GameState newState);

        // Update the current state
        void Update(float deltaTime);

        // Get current state type
        [[nodiscard]] GameState GetCurrentState() const {
            return m_currentStateType;
        }

        // Check if we should exit
        [[nodiscard]] bool ShouldExit() const {
            return m_currentStateType == GameState::Exit;
        }

      private:
        bool firstUpdate = true;
        GameState m_currentStateType{GameState::MainMenu};
        IState *m_currentState{nullptr};
        std::unordered_map<GameState, std::unique_ptr<IState>> m_states;
    };
} // namespace OpenNFS