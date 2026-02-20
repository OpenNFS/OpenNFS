#pragma once

#include "UIElement.h"

namespace OpenNFS {
    enum class ShapeType { Rectangle, Circle, Triangle };

    class UIShape final : public UIElement {
      public:
        UIShape(ShapeType _shapeType, glm::vec4 const &_colour, float _width, float _height, float _scale, uint32_t _layer,
                glm::vec2 const &_location, std::string id = "");
        void Update(glm::vec2 const &cursorPosition, bool click) override;

        ShapeType shapeType;
        glm::vec4 colour;
        float width;
        float height;
    };
} // namespace OpenNFS