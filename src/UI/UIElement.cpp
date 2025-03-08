#include "UIElement.h"

namespace OpenNFS {
    UIElement::UIElement(UIElementType const _type, float const _scale, uint32_t _layer, glm::vec2 const &location)
        : type(_type), scale(_scale), layer(_layer), location(location) {
    }

    void UIElement::SetOnClick(std::function<void()> const &fn) {
        onClickFunction = fn;
    }
} // namespace OpenNFS