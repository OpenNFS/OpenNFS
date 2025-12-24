#include "GameStateManager.h"

#include "../Util/Logger.h"

namespace OpenNFS {
    GameStateManager::GameStateManager() = default;

    void GameStateManager::RegisterState(GameState const stateType, std::unique_ptr<IState> state) {
        m_states[stateType] = std::move(state);
    }

    void GameStateManager::TransitionTo(GameState const newState) {
        if (newState == m_currentStateType && !firstUpdate) {
            return;
        }

        firstUpdate = false;

        // Exit current state if we have one
        if (m_currentState != nullptr) {
            LOG(DEBUG) << "Exiting state: " << magic_enum::enum_name(m_currentStateType);
            m_currentState->OnExit();
        }

        // Find and enter new state
        auto const it = m_states.find(newState);
        if (it != m_states.end()) {
            m_currentStateType = newState;
            m_currentState = it->second.get();
            LOG(DEBUG) << "Entering state: " << magic_enum::enum_name(m_currentStateType);
            m_currentState->OnEnter();
        } else if (newState == GameState::Exit) {
            // No special state handler for this, just signal ShouldExit in main loop from this variable
            m_currentStateType = newState;
        } else {
            LOG(FATAL) << "Attempted to transition to unregistered state: " << magic_enum::enum_name(newState);
        }
    }

    void GameStateManager::Update(float const deltaTime) {
        if (m_currentState != nullptr) {
            m_currentState->OnUpdate(deltaTime);

            // Check if state wants to transition
            GameState const nextState = m_currentState->GetNextState();
            if (nextState != m_currentStateType) {
                TransitionTo(nextState);
            }
        }
    }
} // namespace OpenNFS