#pragma once

#include <map>
#include <string>

#include "../Input/InputManager.h"
#include "../Renderer/UIRenderer.h"
#include "UIElement.h"
#include "UIFont.h"
#include "UILayoutLoader.h"
#include "UIResource.h"

namespace OpenNFS {
    class UIManager {
      public:
        UIManager();
        // Constructor that accepts custom callbacks and layout path
        UIManager(std::string const &layoutPath, UILayoutLoader::CallbackRegistry const &callbacks);
        ~UIManager();
        void Update(InputManager const &inputManager);

        // TODO: Temporary structure
        std::vector<std::unique_ptr<UIElement>> m_uiElements;

      private:
        std::map<std::string, UIResource> m_menuResourceMap;
        std::map<std::string, UIFont> m_fontMap;
        UIRenderer m_uiRenderer;

        // Setup default callbacks for UI elements (used by default constructor)
        static UILayoutLoader::CallbackRegistry SetupDefaultCallbacks();

        // Common initialization logic
        void Initialize(std::string const &layoutPath, UILayoutLoader::CallbackRegistry const &callbacks);
    };
} // namespace OpenNFS
