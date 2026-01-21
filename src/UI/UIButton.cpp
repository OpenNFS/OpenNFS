#include "UIButton.h"

namespace OpenNFS {
    UIButton::UIButton(UIResource const &_resource, std::string const &_text, glm::vec4 const &_textColour, glm::vec4 const &_hoverColour,
                       float const _scale, uint32_t const _layer, glm::vec2 const &_location, glm::vec2 const &_textOffset, std::string id,
                       std::string const &_fontName)
        : UIElement(UIElementType::Button, _scale, _layer, _location, id), resource(_resource), text(_text), textColour(_textColour),
          originalTextColour(_textColour), textHoverColour(_hoverColour), textOffset(_textOffset), fontName(_fontName) {
    }

    void UIButton::Update(glm::vec2 const &cursorPosition, bool const click) {
        if (cursorPosition.x >= location.x && cursorPosition.x <= location.x + (resource.width * scale) && cursorPosition.y >= location.y &&
            cursorPosition.y <= location.y + (resource.height * scale)) {
            if (click) {
                onClickFunction();
            } else {
                onHoverFunction();
                textColour = textHoverColour;
            }
            return;
        }
        textColour = originalTextColour;
    }
} // namespace OpenNFS