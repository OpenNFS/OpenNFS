//
// Created by Amrik on 17/03/2019.
//

#pragma once

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <json.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../Shaders/FontShader.h"
#include "../Shaders/MenuShader.h"
#include "../Util/Logger.h"

using json = nlohmann::json;

struct MenuResource {
    GLuint textureID;
    int width;
    int height;
};

struct Character {
    GLuint     textureID;  // ID handle of the glyph texture
    glm::ivec2 Size;       // Size of glyph
    glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
    GLuint     Advance;    // Offset to advance to next glyph
};

class MenuRenderer {
private:
    // Maps to ease opengl display of character/menu resource
    std::map<GLchar, Character> characters;
    std::map<std::string, MenuResource> menuResources;
    // OpenGL data
    GLuint fontQuadVAO, fontQuadVBO, menuQuadVAO, menuQuadVBO;
    glm::mat4 projectionMatrix;
    // Render shaders
    FontShader fontShader;
    MenuShader menuShader;

    static std::map<std::string, MenuResource> loadResources(const std::string &resourceFile);
    static GLuint loadImage(const std::string &imagePath, int *width, int *height);
public:
    explicit MenuRenderer();
    ~MenuRenderer();
    void render();
    void renderText(const std::string &text, GLint layer, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 colour);
    // Render resource using original resource scale at current resolution
    void renderResource(const std::string &resourceID, GLint layer, GLfloat x, GLfloat y);
    void renderResource(const std::string &resourceID, GLint layer, GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat scale);
};

