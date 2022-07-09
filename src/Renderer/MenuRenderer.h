#pragma once

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Shaders/FontShader.h"
#include "../Shaders/MenuShader.h"
#include "../Util/Logger.h"
#include "../Util/ImageLoader.h"

struct MenuResource {
    GLuint textureID;
    int width;
    int height;
};

struct Character {
    GLuint textureID;   // ID handle of the glyph texture
    glm::ivec2 size;    // Size of glyph
    glm::ivec2 bearing; // Offset from baseline to left/top of glyph
    GLuint advance;     // Offset to advance to next glyph
};

class MenuRenderer {
private:
    // Maps to ease opengl display of character/menu resource
    std::map<GLchar, Character> m_characterMap;
    std::map<std::string, MenuResource> m_menuResourceMap;
    // OpenGL data
    GLuint m_fontQuadVAO, m_fontQuadVBO, m_menuQuadVAO, m_menuQuadVBO;
    glm::mat4 m_projectionMatrix;
    // Render shaders
    FontShader m_fontShader;
    MenuShader m_menuShader;

    static std::map<std::string, MenuResource> LoadResources(const std::string &resourceFile);

public:
    explicit MenuRenderer();
    ~MenuRenderer();
    void Render();
    void RenderText(const std::string &text, GLint layer, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 colour);
    // Render resource using original resource scale at current resolution
    void RenderResource(const std::string &resourceID, GLint layer, GLfloat x, GLfloat y, GLfloat scale);
    void RenderResource(const std::string &resourceID, GLint layer, GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat scale);
};
