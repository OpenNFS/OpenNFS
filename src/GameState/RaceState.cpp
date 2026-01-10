#include "RaceState.h"

namespace OpenNFS {
    RaceState::RaceState(GameContext &context) : m_context(context) {
    }

    void RaceState::OnEnter() {
        // Validate asset data
        CHECK_F(m_context.loadedAssets.trackTag != NFSVersion::UNKNOWN, "Unknown track type!");

        // Load track and car
        m_currentTrack = TrackLoader::Load(m_context.loadedAssets.trackTag, m_context.loadedAssets.track);
        m_currentCar = CarLoader::LoadCar(m_context.loadedAssets.carTag, m_context.loadedAssets.car);

        // Create race session
        m_raceSession = std::make_unique<RaceSession>(m_context.window, m_context.logger, m_context.installedNFS, m_currentTrack,
                                                      m_currentCar, m_context.loadedAssets);
        m_raceActive = true;
        m_nextState = GameState::Race;
    }

    void RaceState::OnUpdate(float const deltaTime) {
        if (m_raceSession && m_raceActive) {
            m_raceSession->Update(deltaTime);

            // Check if race should end
            if (m_raceSession->IsFinished()) {
                LOG(INFO) << "Race finished, returning to main menu";
                m_raceActive = false;
                m_nextState = GameState::MainMenu;
            }
            if (m_raceSession->AssetChanged()) {
                m_raceActive = false;
                m_nextState = GameState::RaceLoad;
                LOG(INFO) << "Asset changed, reloading race with: " << m_context.loadedAssets.to_string();
            }
        }
    }

    void RaceState::OnExit() {
        m_raceSession.reset();
        m_raceActive = false;
    }

    GameState RaceState::GetNextState() const {
        return m_nextState;
    }
} // namespace OpenNFS