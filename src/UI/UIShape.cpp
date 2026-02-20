#include "UIShape.h"

namespace OpenNFS {
    UIShape::UIShape(ShapeType const _shapeType, glm::vec4 const &_colour, float _width, float _height, float _scale, uint32_t _layer,
                     glm::vec2 const &_location, std::string _id)
        : UIElement(UIElementType::Shape, _scale, _layer, _location, std::move(_id)),
          shapeType(_shapeType),
          colour(_colour),
          width(_width),
          height(_height) {
    }

    void UIShape::Update(glm::vec2 const & cursorPosition, bool click) {
        // Shapes are non-interactive
    }
} // namespace OpenNFS