#include "UIImage.h"

namespace OpenNFS {
    UIImage::UIImage(UIResource const &_resource, float const _scale, uint32_t const _layer, glm::vec2 const &_location)
        : UIElement(UIElementType::Image, _scale, _layer, _location), resource(_resource) {
    }

    void UIImage::Update(glm::vec2 const &cursorPosition, bool const click) {
        if (cursorPosition.x >= location.x && cursorPosition.x <= location.x + resource.height && cursorPosition.y >= location.y &&
            cursorPosition.y <= location.y + resource.width) {
            if (click) {
                LOG(INFO) << "Clicked";
            } else {
                LOG(INFO) << "Hover";
            }
        }
    }
} // namespace OpenNFS
