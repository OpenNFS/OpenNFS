#include "RaceLoadState.h"

namespace OpenNFS {
    RaceLoadState::RaceLoadState(GameContext &context) : m_context(context) {
    }

    void RaceLoadState::OnEnter() {
        // Validate asset data
        CHECK_F(m_context.loadedAssets.trackTag != NFSVersion::UNKNOWN, "Unknown track type!");
        CHECK_F(m_context.loadedAssets.carTag != NFSVersion::UNKNOWN, "Unknown car type!");

        // TODO: Load track/car image and display w/ UI Manager + progress bar
    }

    void RaceLoadState::OnUpdate(float const deltaTime) {
        // Then go to the race
        m_nextState = GameState::Race;
    }

    void RaceLoadState::OnExit() {
    }

    GameState RaceLoadState::GetNextState() const {
        return m_nextState;
    }
} // namespace OpenNFS