#include "UIManager.h"

#include <json/json.hpp>

#include "../Util/ImageLoader.h"

namespace OpenNFS {
    using json = nlohmann::json;

    UIManager::UIManager() {
        m_menuResourceMap = LoadResources("../resources/ui/menu/resources.json");
        LOG(INFO) << m_menuResourceMap.size() << " UI resources loaded successfully";

        auto testButton = std::make_unique<UIButton>(m_menuResourceMap["onfsLogo"], "TEST", glm::vec4(0.5, 0.5, 0, 1), 0.1f, 2, glm::vec2(Config::get().resX/2,Config::get().resY/2));
        auto onfsLogoImage = std::make_unique<UIImage>(m_menuResourceMap["onfsLogo"], 0.1f, 0, glm::vec2(Config::get().resX - 75, 5));
        auto onfsVersionText = std::make_unique<UITextField>("OpenNFS v" + ONFS_VERSION + " Pre Alpha", glm::vec4(0.6, 0.6, 0.6, 1.0), 0.2f,
                                                             0, glm::vec2(Config::get().resX - 270, 35));

        // TODO: Do depth sorting here
        //m_uiElements.push_back(std::move(testButton));
        m_uiElements.push_back(std::move(onfsLogoImage));
        m_uiElements.push_back(std::move(onfsVersionText));
    }
    UIManager::~UIManager() {
        for (auto &resource : m_menuResourceMap | std::views::values) {
            glDeleteTextures(1, &resource.textureID);
        }
    }
    void UIManager::Render() {
        m_uiRenderer.BeginRenderPass();

        for (auto const &uiElement : m_uiElements) {
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

    std::map<std::string, UIResource> UIManager::LoadResources(std::string const &resourceFile) {
        // Read the resource JSON file
        std::ifstream jsonFile(resourceFile);
        CHECK_F(jsonFile.is_open(), "Couldn't open menu resource file %s", resourceFile.c_str());

        std::map<std::string, UIResource> resources;
        json resourcesJson;
        jsonFile >> resourcesJson;

        for (auto &el : resourcesJson["resources"].items()) {
            std::string elementName = el.value()["name"];

            // Load the image into the GPU and get corresponding handle
            int width, height;
            GLuint const textureID{ImageLoader::LoadImage(el.value()["path"], &width, &height, GL_CLAMP_TO_EDGE, GL_LINEAR)};
            // Now store menu resource for later use
            UIResource menuResource{textureID, width, height};
            resources.insert(std::pair(elementName, menuResource));
        }
        jsonFile.close();

        return resources;
    }
} // namespace OpenNFS
