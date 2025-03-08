#include "UIElement.h"

namespace OpenNFS {

    UIElement::UIElement(UIElementType const _type, float const _scale, uint32_t _layer, glm::vec2 const &location)
        : type(_type), scale(_scale), layer(_layer), location(location) {
    }

    void UIElement::SetOnClick(std::function<void()> const &fn) {
        onClickFunction = fn;
    }

    UIButton::UIButton(UIResource const &_resource,
                       std::string const &_text,
                       glm::vec4 const &_textColour,
                       float const _scale,
                       uint32_t _layer,
                       glm::vec2 const &_location)
        : UIElement(UIElementType::Button, _scale, _layer, _location), resource(_resource), text(_text), textColour(_textColour) {
    }

    void UIButton::Update() {
    }

    UITextField::UITextField(
        std::string const &_text, glm::vec4 const &_textColour, float const _scale, uint32_t const _layer, glm::vec2 const &_location)
        : UIElement(UIElementType::TextField, _scale, _layer, _location), text(_text), textColour(_textColour) {
    }

    void UITextField::Update() {
    }

    UIImage::UIImage(UIResource const &_resource, float const _scale, uint32_t const _layer, glm::vec2 const &_location)
        : UIElement(UIElementType::Image, _scale, _layer, _location), resource(_resource) {
    }

    void UIImage::Update() {
    }

} // namespace OpenNFS