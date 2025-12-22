#pragma once

#include <map>
#include <string>

#include "../Input/InputManager.h"
#include "../Renderer/UIRenderer.h"
#include "UIElement.h"
#include "UIResource.h"

namespace OpenNFS {
    class UIManager {
      public:
        UIManager();
        ~UIManager();
        void Update(InputManager const &inputManager);

        // TODO: Temporary structure
        std::vector<std::unique_ptr<UIElement>> m_uiElements;

      private:
        std::map<std::string, UIResource> m_menuResourceMap;
        UIRenderer m_uiRenderer;
    };
} // namespace OpenNFS
