#include "UIElement.h"

namespace OpenNFS {
    UIElement::UIElement(UIElementType const _type, float const _scale, uint32_t const _layer, glm::vec2 const &location, std::string id)
        : type(_type), scale(_scale), layer(_layer), location(location), id(id), onClickFunction([] {}), onHoverFunction([] {}) {
    }

    void UIElement::SetOnClick(std::function<void()> const &fn) {
        onClickFunction = fn;
    }

    void UIElement::SetOnHover(std::function<void()> const &fn) {
        onHoverFunction = fn;
    }
} // namespace OpenNFS