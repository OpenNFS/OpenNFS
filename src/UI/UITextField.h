#pragma once

#include <string>

#include "UIElement.h"

namespace OpenNFS {
    class UITextField final : public UIElement {
      public:
        UITextField(std::string const &_text, glm::vec4 const &_textColour, float _scale, uint32_t _layer, glm::vec2 const &_location);
        void Update(glm::vec2 const &cursorPosition, bool click) override;

        std::string text;
        glm::vec4 textColour;
    };
} // namespace OpenNFS
