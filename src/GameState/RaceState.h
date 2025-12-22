#pragma once

#include <memory>

#include "../Loaders/CarLoader.h"
#include "../Loaders/TrackLoader.h"
#include "../Race/RaceSession.h"
#include "GameState.h"
#include "MainMenuState.h"

namespace OpenNFS {
    class RaceState : public IState {
      public:
        explicit RaceState(GameContext &context);

        void OnEnter() override;
        void OnUpdate(float deltaTime) override;
        void OnExit() override;
        GameState GetNextState() const override;

      private:
        GameContext &m_context;
        std::unique_ptr<RaceSession> m_raceSession;
        std::shared_ptr<Track> m_currentTrack;
        std::shared_ptr<Car> m_currentCar;
        GameState m_nextState{GameState::Race};
        bool m_raceActive{false};
    };
} // namespace OpenNFS