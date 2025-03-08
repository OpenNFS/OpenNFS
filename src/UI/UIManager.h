#pragma once

#include "UIElement.h"
#include "../Renderer/UIRenderer.h"

#include <map>
#include <string>

namespace OpenNFS {
    class UIManager {
        public:
        UIManager();
        ~UIManager();
        void Render();

        // TODO: Temporary structure
        std::vector<std::unique_ptr<UIElement>> m_uiElements;

        static std::map<std::string, UIResource> LoadResources(std::string const &resourceFile);
    private:
        std::map<std::string, UIResource> m_menuResourceMap;
        UIRenderer m_uiRenderer;
    };
} // namespace OpenNFS
