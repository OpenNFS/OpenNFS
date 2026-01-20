#pragma once

#include "UIElement.h"
#include "UIResource.h"

namespace OpenNFS {
    class UIImage final : public UIElement {
      public:
        UIImage(UIResource const &_resource, float _scale, uint32_t _layer, glm::vec2 const &_location, std::string id = "");
        void Update(glm::vec2 const &cursorPosition, bool click) override;

        UIResource const &resource;
    };
} // namespace OpenNFS
