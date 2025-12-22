#include "UIManager.h"

#include <ranges>

#include "UIButton.h"
#include "UIFont.h"
#include "UIImage.h"
#include "UIResource.h"
#include "UITextField.h"

namespace OpenNFS {
    UIManager::UIManager() {
        // Load fonts
        m_fontMap = UIFont::LoadFonts("../resources/ui/fonts.json");
        LOG(INFO) << m_fontMap.size() << " UI fonts loaded successfully";
        CHECK_F(m_uiRenderer.GenerateAtlases(m_fontMap), "Failed to generate atlases from font map");

        // Load image resources
        m_menuResourceMap = UIResource::LoadResources("../resources/ui/menu/resources.json");
        LOG(INFO) << m_menuResourceMap.size() << " UI resources loaded successfully";

        // TODO: Load these from JSON too
        auto onfsLogoImage = std::make_unique<UIImage>(m_menuResourceMap["onfsLogo"], 0.1f, 0, glm::vec2(Config::get().resX - 75, 5));
        auto onfsVersionText = std::make_unique<UITextField>("OpenNFS v" + ONFS_VERSION + " Pre Alpha", glm::vec4(0.6, 0.6, 0.6, 1.0), 0.2f,
                                                             0, glm::vec2(Config::get().resX - 270, 35), "default");
        auto const bgBackgroundGrille = std::make_unique<UIImage>(m_menuResourceMap["backgroundPattern"], 1.f, 1, glm::vec2(0, 0));

        // TODO: Hardcode all OnClick functions, the JSON will encode a constant that looks up the correct callback when parsing
        onfsLogoImage->SetOnClick([] { LOG(INFO) << "Clicked onfsLogo"; });
        bgBackgroundGrille->SetOnClick([] { LOG(INFO) << "Clicked backgroundGrille"; });

        // m_uiElements.push_back(std::move(bgBackgroundGrille));
        m_uiElements.push_back(std::move(onfsLogoImage));
        m_uiElements.push_back(std::move(onfsVersionText));
    }
    UIManager::~UIManager() {
        for (auto &resource : m_menuResourceMap | std::views::values) {
            glDeleteTextures(1, &resource.textureID);
        }
    }

    auto UIManager::Update(InputManager const &inputManager) -> void {
        // Cursor Y needs to be inverted + coordinates need normalising
        float const windowToResRatioX{(float)Config::get().resX / (float)Config::get().windowSizeX};
        float const windowToResRatioY{(float)Config::get().resY / (float)Config::get().windowSizeY};
        glm::vec2 const cursorPosition{inputManager.cursorX * windowToResRatioX, Config::get().resY - (inputManager.cursorY * windowToResRatioY)};

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
