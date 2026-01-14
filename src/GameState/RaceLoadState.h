#pragma once

#include <memory>

#include "GameState.h"
#include "MainMenuState.h"

namespace OpenNFS {
    class RaceLoadState : public IState {
      public:
        explicit RaceLoadState(GameContext &context);

        void OnEnter() override;
        void OnUpdate(float deltaTime) override;
        void OnExit() override;
        GameState GetNextState() const override;

      private:
        GameContext &m_context;
        GameState m_nextState{GameState::Race};
    };
} // namespace OpenNFS