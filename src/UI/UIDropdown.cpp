#include "UIDropdown.h"

namespace OpenNFS {
    UIDropdown::UIDropdown(UIResource const &_resource, UIResource const &_entryResource, std::string const &_text, glm::vec4 const &_textColour, glm::vec4 const &_hoverColour,
                       float const _scale, uint32_t const _layer, glm::vec2 const &_location, glm::vec2 const &_textOffset, std::vector<std::string> _entries, std::string _id,
                       std::string const &_fontName)
        : UIElement(UIElementType::Dropdown, _scale, _layer, _location, _id), resource(_resource), entryResource(_entryResource), text(_text), textColour(_textColour),
          originalTextColour(_textColour), textHoverColour(_hoverColour), textOffset(_textOffset), fontName(_fontName), entries(_entries) {
            for(size_t i = 0; i < entries.size(); i++)
                entryTextColour.push_back(textColour);
            if (text.empty() && entries.size() > 0)
                text = entries[0];
    }

    void UIDropdown::Update(glm::vec2 const &cursorPosition, bool const click) {
        bool hoverOnAnyPart = false;
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
                            onHoverFunction();
                            hoverOnAnyPart = true;
                        }
                    }
                    else {
                        entryTextColour[i] = originalTextColour;
                    }
            }
        }
        bool hover = cursorPosition.x >= location.x && cursorPosition.x <= location.x + (resource.width * scale) && cursorPosition.y >= location.y &&
            cursorPosition.y <= location.y + (resource.height * scale);
        
        if (hover) {
            if (click)
                isOpened = true;
            textColour = textHoverColour;
            hoverOnAnyPart = true;
        } else {
            textColour = originalTextColour;
        }

        // If the mouse is in the square of the button and the last option, keep the dropdown open
        // If the mouse is outside this area, close it
        bool hoverBetweenButtonAndLastOption = cursorPosition.x >= location.x && cursorPosition.x <= location.x + (resource.width * scale) + (entryResource.width * scale) &&
            cursorPosition.y >= location.y - (entryResource.height * (float) (entries.size() - 1) * scale) && cursorPosition.y <= location.y;
        if (hoverBetweenButtonAndLastOption) {
            // Close if we click under the dropdown menu button, but not on an entry in it
            if (click)
                isOpened = false;
        } else if (!hoverOnAnyPart)
            isOpened = false;
    }

    void UIDropdown::AddEntry(std::string entry) {
        if (text.empty())
            text = entry;
        entries.push_back(entry);
        entryTextColour.push_back(textColour);
    }
} // namespace OpenNFS