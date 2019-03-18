//
// Created by Amrik on 17/03/2019.
//

#pragma once

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../Shaders/FontShader.h"
#include "../Util/Logger.h"

class MenuRenderer {
private:
    GLuint VAO, VBO;

    struct Character {
        GLuint     textureID;  // ID handle of the glyph texture
        glm::ivec2 Size;       // Size of glyph
        glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
        GLuint     Advance;    // Offset to advance to next glyph
    };

    std::map<GLchar, Character> Characters;
    FontShader fontShader;
    //MenuShader menuShader;
public:
    explicit MenuRenderer();
    void Render();
    void renderText(const std::string &text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 colour);
};

