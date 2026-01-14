#pragma once

#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace OpenNFS {
    enum class UIElementType {
        Button,
        TextField,
        Image
    };

    class UIElement {
      public:
        UIElement(UIElementType _type, float _scale, uint32_t _layer, glm::vec2 const &location);
        void SetOnClick(std::function<void()> const &fn);
        void SetOnHover(std::function<void()> const &fn);

        virtual void Update(glm::vec2 const &cursorPosition, bool click) = 0;
        virtual ~UIElement() = default;

        UIElementType type;
        float scale;
        uint32_t layer;
        glm::vec2 location;

      protected:
        std::function<void()> onClickFunction;
        std::function<void()> onHoverFunction;
    };
} // namespace OpenNFS
