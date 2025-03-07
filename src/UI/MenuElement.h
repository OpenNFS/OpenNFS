#pragma once

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace OpenNFS {
    struct MenuResource {
        GLuint textureID;
        int width;
        int height;
    };

    enum class MenuAction {
        None,
        Exit,
    };

    enum MenuScreen {
    };

    class MenuElement {
        MenuElement(std::string idleAssetRef,
                    std::string hoverAssetRef,
                    MenuAction _onClick,
                    std::string _tooltipText,
                    std::string _mainText,
                    uint32_t _layer);
        MenuResource idleAsset;
        MenuResource hoverAsset;
        MenuAction onClick;
        std::string tooltipText;
        std::string mainText;
        glm::vec4 textColour;
        uint32_t layer;
        glm::vec2 location;
    };
} // namespace OpenNFS
