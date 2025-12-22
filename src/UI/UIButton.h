#pragma once

#include "UIElement.h"
#include "UIResource.h"

namespace OpenNFS {
    class UIButton final : public UIElement {
      public:
        UIButton(UIResource const &_resource, std::string const &_text, glm::vec4 const &_textColour, glm::vec4 const &_textHoverColour,
                 float _scale, uint32_t _layer, glm::vec2 const &_location, glm::vec2 const &_textOffset,
                 std::string const &_fontName = "default");
        void Update(glm::vec2 const &cursorPosition, bool click) override;

        UIResource const &resource;
        std::string text;
        glm::vec4 textColour;
        glm::vec4 originalTextColour;
        glm::vec4 textHoverColour;
        glm::vec2 textOffset;
        std::string fontName;
    };
} // namespace OpenNFS
