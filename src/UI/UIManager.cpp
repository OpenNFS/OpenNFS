#include "UIManager.h"

#include <ranges>

#include "UIButton.h"
#include "UIFont.h"
#include "UIImage.h"
#include "UILayoutLoader.h"
#include "UIResource.h"
#include "UITextField.h"

namespace OpenNFS {
    UILayoutLoader::CallbackRegistry UIManager::SetupCallbacks() {
        UILayoutLoader::CallbackRegistry callbacks;

        // Register all UI callbacks here
        callbacks["onLogoClick"] = [] { LOG(INFO) << "Clicked onfsLogo"; };
        callbacks["onBackgroundClick"] = [] { LOG(INFO) << "Clicked backgroundGrille"; };

        return callbacks;
    }

    UIManager::UIManager() {
        // Load fonts
        m_fontMap = UIFont::LoadFonts("../resources/ui/fonts/fonts.json");
        LOG(INFO) << m_fontMap.size() << " UI fonts loaded successfully";
        CHECK_F(m_uiRenderer.GenerateAtlases(m_fontMap), "Failed to generate atlases from font map");

        // Load image resources
        m_menuResourceMap = UIResource::LoadResources("../resources/ui/menu/resources.json");
        LOG(INFO) << m_menuResourceMap.size() << " UI resources loaded successfully";

        // Load UI layout from JSON
        UILayoutLoader layoutLoader(m_menuResourceMap, m_fontMap);
        auto const callbacks = SetupCallbacks();
        m_uiElements = layoutLoader.LoadLayout("../resources/ui/menu/layout/raceOverlay.json", callbacks);
    }

    UIManager::~UIManager() {
        for (auto &resource : m_menuResourceMap | std::views::values) {
            glDeleteTextures(1, &resource.textureID);
        }
    }

    auto UIManager::Update(InputManager const &inputManager) -> void {
        // Cursor Y needs to be inverted + coordinates need normalising
        float const windowToResRatioX{static_cast<float>(Config::get().resX) / static_cast<float>(Config::get().windowSizeX)};
        float const windowToResRatioY{static_cast<float>(Config::get().resY) / static_cast<float>(Config::get().windowSizeY)};
        glm::vec2 const cursorPosition{inputManager.cursorX * windowToResRatioX,
                                       Config::get().resY - (inputManager.cursorY * windowToResRatioY)};

        m_uiRenderer.BeginRenderPass();
        for (auto const &uiElement : m_uiElements) {
            uiElement->Update(cursorPosition, inputManager.mouseLeft);

            switch (uiElement->type) {
            case UIElementType::Button:
                m_uiRenderer.RenderButton(dynamic_cast<UIButton *>(uiElement.get()));
                break;
            case UIElementType::TextField:
                m_uiRenderer.RenderTextField(dynamic_cast<UITextField *>(uiElement.get()));
                break;
            case UIElementType::Image:
                m_uiRenderer.RenderImage(dynamic_cast<UIImage *>(uiElement.get()));
                break;
            }
        }
        m_uiRenderer.EndRenderPass();
    }
} // namespace OpenNFS
