#pragma once

#include <vector>

#include "UIElement.h"
#include "UIResource.h"

namespace OpenNFS {
    class UIDropdown final : public UIElement {
      public:
        UIDropdown(UIResource const &_resource, UIResource const &_entryResource, std::string const &_text, glm::vec4 const &_textColour, glm::vec4 const &_textHoverColour,
                 float _scale, uint32_t _layer, glm::vec2 const &_location, glm::vec2 const &_textOffset,
                 std::string const &_fontName = "default");
        void Update(glm::vec2 const &cursorPosition, bool click) override;

        UIResource const &resource;
        UIResource const &entryResource;
        std::string text;
        glm::vec4 textColour;
        glm::vec4 originalTextColour;
        glm::vec4 textHoverColour;
        glm::vec2 textOffset;
        std::string fontName;
        bool isOpened = false;
        std::vector<std::string> entries;
        std::vector<glm::vec4> entryTextColour;
    };
} // namespace OpenNFS
