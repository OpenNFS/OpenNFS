#include "MainMenuState.h"

namespace OpenNFS {
    MainMenuState::MainMenuState(GameContext &context) : m_context(context), m_inputManager(context.window) {
    }

    void MainMenuState::OnEnter() {
        LOG(INFO) << "Entering Main Menu";

        // Setup callbacks for the main menu
        UILayoutLoader::CallbackRegistry callbacks;
        callbacks["onStartRace"] = [this]() { OnStartRaceClicked(); };
        callbacks["onExit"] = [this]() { m_nextState = GameState::Exit; };

        // Create UI manager with main menu layout
        m_uiManager = std::make_unique<UIManager>("../resources/ui/menu/layout/mainMenu.json", callbacks);

        // Reset next state
        m_nextState = GameState::MainMenu;
    }

    void MainMenuState::OnUpdate(float const deltaTime) {
        // Update input
        m_inputManager.Scan();

        // Handle escape key to exit
        if (m_inputManager.escape) {
            m_nextState = GameState::Exit;
        }

        // Update and render UI
        m_uiManager->Update(m_inputManager);
    }

    void MainMenuState::OnExit() {
        LOG(INFO) << "Exiting Main Menu";
        m_uiManager.reset();
    }

    GameState MainMenuState::GetNextState() const {
        return m_nextState;
    }

    void MainMenuState::OnStartRaceClicked() {
        LOG(INFO) << "Start Race button clicked";
        m_nextState = GameState::Race;
    }
} // namespace OpenNFS