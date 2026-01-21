#include "UIManager.h"

#include <ranges>

#include "UIButton.h"
#include "UIDropdown.h"
#include "UIFont.h"
#include "UIImage.h"
#include "UILayoutLoader.h"
#include "UIResource.h"
#include "UITextField.h"

namespace OpenNFS {
    UILayoutLoader::CallbackRegistry UIManager::SetupDefaultCallbacks() {
        UILayoutLoader::CallbackRegistry callbacks;

        // Register all UI callbacks here
        callbacks["onLogoClick"] = [] { LOG(INFO) << "Clicked onfsLogo"; };
        callbacks["onBackgroundClick"] = [] { LOG(INFO) << "Clicked backgroundGrille"; };

        return callbacks;
    }

    void UIManager::Initialize(std::string const &layoutPath, UILayoutLoader::CallbackRegistry const &callbacks) {
        // Load fonts
        m_fontMap = UIFont::LoadFonts("../resources/ui/fonts/fonts.json");
        LOG(INFO) << m_fontMap.size() << " UI font(s) loaded successfully";
        CHECK_F(m_uiRenderer.GenerateAtlases(m_fontMap), "Failed to generate atlases from font map");

        // Load image resources
        m_menuResourceMap = UIResource::LoadResources("../resources/ui/menu/resources.json");
        LOG(INFO) << m_menuResourceMap.size() << " UI resources loaded successfully";

        // Load UI layout from JSON
        UILayoutLoader const layoutLoader(m_menuResourceMap, m_fontMap);
        m_uiElements = layoutLoader.LoadLayout(layoutPath, callbacks);
    }

    UIManager::UIManager() {
        Initialize("../resources/ui/menu/layout/raceOverlay.json", SetupDefaultCallbacks());
    }

    UIManager::UIManager(std::string const &layoutPath, UILayoutLoader::CallbackRegistry const &callbacks) {
        Initialize(layoutPath, callbacks);
    }

    UIManager::~UIManager() {
        for (auto &resource : m_menuResourceMap | std::views::values) {
            glDeleteTextures(1, &resource.textureID);
        }
    }

    auto UIManager::Update(InputManager const &inputManager) -> void {
        m_uiRenderer.BeginRenderPass();
        for (auto const &uiElement : m_uiElements) {
            uiElement->Update({inputManager.uiCursorX, inputManager.uiCursorY}, inputManager.mouseLeft);

            switch (uiElement->type) {
            case UIElementType::Button:
                m_uiRenderer.RenderButton(dynamic_cast<UIButton *>(uiElement.get()));
                break;
            case UIElementType::Dropdown:
                m_uiRenderer.RenderDropdown(dynamic_cast<UIDropdown *>(uiElement.get()));
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
    std::shared_ptr<UIElement> UIManager::GetElementWithID(std::string _id) {
        for (auto element : m_uiElements) {
            if (element.get()->id == _id) {
                return element;
            }
        }
        return nullptr;
    }
} // namespace OpenNFS
