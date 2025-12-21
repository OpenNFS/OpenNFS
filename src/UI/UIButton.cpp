#include "UIButton.h"

namespace OpenNFS {
    UIButton::UIButton(UIResource const &_resource, std::string const &_text, glm::vec4 const &_textColour, float const _scale,
                       uint32_t const _layer, glm::vec2 const &_location)
        : UIElement(UIElementType::Button, _scale, _layer, _location), resource(_resource), text(_text), textColour(_textColour) {
    }

    void UIButton::Update(glm::vec2 const &cursorPosition, bool click) {
    }
} // namespace OpenNFS