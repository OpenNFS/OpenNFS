#include "UITextField.h"

namespace OpenNFS {
    UITextField::UITextField(
        std::string const &_text, glm::vec4 const &_textColour, float const _scale, uint32_t const _layer, glm::vec2 const &_location)
        : UIElement(UIElementType::TextField, _scale, _layer, _location), text(_text), textColour(_textColour) {
    }

    void UITextField::Update(glm::vec2 const &cursorPosition, bool click) {
    }
} // namespace OpenNFS