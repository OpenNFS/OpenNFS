#pragma once

namespace OpenNFS {
    enum class GameState {
        MainMenu,
        Race,
        Paused,
        Exit
    };

    // Forward declarations
    class GameStateManager;

    // Base interface for all game states
    class IState {
      public:
        virtual ~IState() = default;

        // Called when entering this state
        virtual void OnEnter() = 0;

        // Called every frame while in this state
        virtual void OnUpdate(float deltaTime) = 0;

        // Called when exiting this state
        virtual void OnExit() = 0;

        // Returns the next state to transition to (or current state to stay)
        [[nodiscard]] virtual GameState GetNextState() const = 0;
    };
} // namespace OpenNFS