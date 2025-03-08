#pragma once

#include <GL/glew.h>
#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace OpenNFS {
    struct UIResource {
        GLuint textureID;
        int width;
        int height;
    };

    enum class UIElementType {
        Button,
        TextField,
        Image
    };

    class UIElement {
      public:
        UIElement(UIElementType _type, float _scale, uint32_t _layer, glm::vec2 const &location);
        void SetOnClick(std::function<void()> const &fn);

        virtual void Update() = 0;
        virtual ~UIElement() = default;

      UIElementType type;
        float scale;
        uint32_t layer;
        glm::vec2 location;
      private:
        std::function<void()> onClickFunction;
    };

    class UIButton final : public UIElement {
      public:
        UIButton(UIResource const &_resource,
                 std::string const &_text,
                 glm::vec4 const &_textColour,
                 float _scale,
                 uint32_t _layer,
                 glm::vec2 const &_location);
        void Update() override;

        UIResource const &resource;
        std::string text;
        glm::vec4 textColour;
    };

    class UITextField final : public UIElement {
      public:
        UITextField(std::string const &_text, glm::vec4 const &_textColour, float _scale, uint32_t _layer, glm::vec2 const &_location);
        void Update() override;

        std::string text;
        glm::vec4 textColour;
    };

    class UIImage final : public UIElement {
      public:
        UIImage(UIResource const &_resource, float _scale, uint32_t _layer, glm::vec2 const &_location);
        void Update() override;

        UIResource const &resource;
    };
} // namespace OpenNFS
