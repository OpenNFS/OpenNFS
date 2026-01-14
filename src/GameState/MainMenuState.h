#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "../Input/InputManager.h"
#include "../UI/UIManager.h"
#include "../Util/Logger.h"
#include "GameState.h"

namespace OpenNFS {
    // Context shared between all states
    struct GameContext {
        std::shared_ptr<GLFWwindow> window;
        std::shared_ptr<Logger> logger;
        std::vector<NfsAssetList> installedNFS;
        AssetData loadedAssets;
    };

    class MainMenuState : public IState {
      public:
        explicit MainMenuState(GameContext &context);

        void OnEnter() override;
        void OnUpdate(float deltaTime) override;
        void OnExit() override;
        GameState GetNextState() const override;

        // Callback for starting race
        void OnStartRaceClicked();

      private:
        GameContext &m_context;
        InputManager m_inputManager;
        std::unique_ptr<UIManager> m_uiManager;
        GameState m_nextState{GameState::MainMenu};
    };
} // namespace OpenNFS