#include "UIManager.h"

#include <ranges>

#include "UIButton.h"
#include "UIImage.h"
#include "UIResource.h"
#include "UITextField.h"
#include "../Util/ImageLoader.h"

namespace OpenNFS {
    UIManager::UIManager() {
        m_menuResourceMap = UIResource::LoadResources("../resources/ui/menu/resources.json");
        LOG(INFO) << m_menuResourceMap.size() << " UI resources loaded successfully";

        // TODO: Load these from JSON too
        auto testButton = std::make_unique<UIButton>(m_menuResourceMap["onfsLogo"], "TEST", glm::vec4(0.5, 0.5, 0, 1), 0.1f, 2,
                                                     glm::vec2(Config::get().resX / 2, Config::get().resY / 2));
        auto onfsLogoImage = std::make_unique<UIImage>(m_menuResourceMap["onfsLogo"], 0.1f, 0, glm::vec2(Config::get().resX - 75, 5));
        auto onfsVersionText = std::make_unique<UITextField>("OpenNFS v" + ONFS_VERSION + " Pre Alpha", glm::vec4(0.6, 0.6, 0.6, 1.0), 0.2f,
                                                             0, glm::vec2(Config::get().resX - 270, 35));

        // TODO: Hardcode all OnClick functions, the JSON will encode a constant that looks up the correct callback when parsing
        onfsLogoImage->SetOnClick([](){LOG(INFO) << "Clicked onfsLogo";});
        // m_uiElements.push_back(std::move(testButton));
        m_uiElements.push_back(std::move(onfsLogoImage));
        m_uiElements.push_back(std::move(onfsVersionText));
    }
    UIManager::~UIManager() {
        for (auto &resource : m_menuResourceMap | std::views::values) {
            glDeleteTextures(1, &resource.textureID);
        }
    }

    auto UIManager::Update(InputManager::Inputs const &inputs) -> void {
        // Cursor Y needs to be inverted + coordinates need normalising
        float const windowToResRatioX {(float)Config::get().resX/(float)Config::get().windowSizeX};
        float const windowToResRatioY {(float)Config::get().resY/(float)Config::get().windowSizeY};
        glm::vec2 const cursorPosition{inputs.cursorX * windowToResRatioX, Config::get().resY - (inputs.cursorY * windowToResRatioY)};

        m_uiRenderer.BeginRenderPass();
        for (auto const &uiElement : m_uiElements) {
            uiElement->Update(cursorPosition, inputs.mouseLeft);

            switch (uiElement->type) {
            case UIElementType::Button:
                m_uiRenderer.RenderButton(static_cast<UIButton *>(uiElement.get()));
                break;
            case UIElementType::TextField:
                m_uiRenderer.RenderTextField(static_cast<UITextField *>(uiElement.get()));
                break;
            case UIElementType::Image:
                m_uiRenderer.RenderImage(static_cast<UIImage *>(uiElement.get()));
                break;
            }
        }
        m_uiRenderer.EndRenderPass();
    }
} // namespace OpenNFS
