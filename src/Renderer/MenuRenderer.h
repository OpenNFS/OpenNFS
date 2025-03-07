#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Shaders/FontShader.h"
#include "../Shaders/MenuShader.h"
#include "../UI/MenuElement.h"
#include "../Util/ImageLoader.h"

namespace OpenNFS {
    struct Character {
        GLuint textureID;   // ID handle of the glyph texture
        glm::ivec2 size;    // Size of glyph
        glm::ivec2 bearing; // Offset from baseline to left/top of glyph
        GLuint advance;     // Offset to advance to next glyph
    };

    class MenuRenderer {
        // Maps to ease opengl display of character/menu resource
        std::map<GLchar, Character> m_characterMap;
        std::map<std::string, MenuResource> m_menuResourceMap;
        // OpenGL data
        GLuint m_fontQuadVAO, m_fontQuadVBO, m_menuQuadVAO, m_menuQuadVBO;
        glm::mat4 m_projectionMatrix{};
        // Render shaders
        FontShader m_fontShader;
        MenuShader m_menuShader;

        static std::map<std::string, MenuResource> LoadResources(std::string const &resourceFile);

      public:
        explicit MenuRenderer();
        ~MenuRenderer();

        void Render();
        void RenderText(std::string const &text, GLint layer, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 colour);

        // Render resource using original resource scale at current resolution
        void RenderResource(MenuResource const& resource, GLint layer, GLfloat x, GLfloat y, GLfloat scale);
        void RenderResource(MenuResource const& resource, GLint layer, GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat scale);
    };
} // namespace OpenNFS
