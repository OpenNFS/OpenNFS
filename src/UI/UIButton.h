#pragma once

#include "UIElement.h"
#include "UIResource.h"

namespace OpenNFS {
    class UIButton final : public UIElement {
      public:
        UIButton(UIResource const &_resource, std::string const &_text, glm::vec4 const &_textColour, float _scale, uint32_t _layer,
                 glm::vec2 const &_location);
        void Update(glm::vec2 const &cursorPosition, bool click) override;

        UIResource const &resource;
        std::string text;
        glm::vec4 textColour;
    };
} // namespace OpenNFS
