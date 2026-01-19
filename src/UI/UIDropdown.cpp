#include "UIDropdown.h"

namespace OpenNFS {
    UIDropdown::UIDropdown(UIResource const &_resource, UIResource const &_entryResource, std::string const &_text, glm::vec4 const &_textColour, glm::vec4 const &_hoverColour,
                       float const _scale, uint32_t const _layer, glm::vec2 const &_location, glm::vec2 const &_textOffset,
                       std::string const &_fontName)
        : UIElement(UIElementType::Dropdown, _scale, _layer, _location), resource(_resource), entryResource(_entryResource), text(_text), textColour(_textColour),
          originalTextColour(_textColour), textHoverColour(_hoverColour), textOffset(_textOffset), fontName(_fontName) {
            // for(size_t i = 0; i < 7; i++)
            //     entryTextColour.push_back(textColour);
    }

    void UIDropdown::Update(glm::vec2 const &cursorPosition, bool const click) {
        if (isOpened) {
            float entryLocationX = location.x + (resource.width * scale);
            for(size_t i = 0; i < entries.size(); i++) {
                float entryLocationY = location.y - (entryResource.height * scale * (float) i);
                if(cursorPosition.x >= entryLocationX && cursorPosition.x <= entryLocationX + (entryResource.width * scale) && cursorPosition.y >= entryLocationY &&
                    cursorPosition.y <= entryLocationY + (entryResource.height * scale)) {
                        if (click) {
                            text = entries[i];
                            onClickFunction();
                        } else {
                            entryTextColour[i] = textHoverColour;
                        }
                    }
                    else {
                        entryTextColour[i] = originalTextColour;
                    }
            }
        }
        bool hover = cursorPosition.x >= location.x && cursorPosition.x <= location.x + (resource.width * scale) && cursorPosition.y >= location.y &&
            cursorPosition.y <= location.y + (resource.height * scale);
        
        if (click) {
            isOpened = hover;
        } else if (hover) {
            onHoverFunction();
            textColour = textHoverColour;
        } else {
            textColour = originalTextColour;
        }
    }
} // namespace OpenNFS