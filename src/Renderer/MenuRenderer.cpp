#include "MenuRenderer.h"

#include <json.hpp>
#include <ft2build.h>

#include FT_FREETYPE_H

using json = nlohmann::json;

MenuRenderer::MenuRenderer() {
    FT_Library ft;
    ASSERT(!FT_Init_FreeType(&ft), "FREETYPE: Could not init FreeType Library");

    FT_Face face;
    ASSERT(!FT_New_Face(ft, "../resources/ui/fonts/earth.ttf", 0, &face), "FREETYPE: Failed to load font");

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (GLubyte c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            LOG(WARNING) << "FREETYPE: Failed to load Glyph";
            continue;
        }
        // Generate texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {textureID, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               (GLuint) face->glyph->advance.x};
        m_characterMap.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Configure VAO/VBO for font texture quads
    glGenVertexArrays(1, &m_fontQuadVAO);
    glGenBuffers(1, &m_fontQuadVBO);
    glBindVertexArray(m_fontQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_fontQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    // Configure VAO/VBO for menu texture quads
    glGenVertexArrays(1, &m_menuQuadVAO);
    glGenBuffers(1, &m_menuQuadVBO);
    glBindVertexArray(m_menuQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_menuQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    // Reset state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    LOG(INFO) << "Glyphs loaded successfully";

    m_menuResourceMap = LoadResources("../resources/ui/menu/resources.json");
    LOG(INFO) << "Menu resources loaded successfully";
}

MenuRenderer::~MenuRenderer() {
    // Lets delete all of the loaded textures
    for (auto it = m_characterMap.begin(); it != m_characterMap.end(); ++it) {
        glDeleteTextures(1, &it->second.textureID);
    }
    for (auto it = m_menuResourceMap.begin(); it != m_menuResourceMap.end(); ++it) {
        glDeleteTextures(1, &it->second.textureID);
    }
}

std::map<std::string, MenuResource> MenuRenderer::LoadResources(const std::string &resourceFile) {
    // Read the resource JSON file
    std::ifstream jsonFile(resourceFile);
    ASSERT(jsonFile.is_open(), "Couldn't open menu resource file " << resourceFile);

    std::map<std::string, MenuResource> resources;
    json resourcesJson;
    jsonFile >> resourcesJson;

    for (auto &el : resourcesJson["resources"].items()) {
        std::string elementName = el.value()["name"];

        // Load the image into the GPU and get corresponding handle
        int width, height;
        GLuint textureID = ImageLoader::LoadImage(el.value()["path"], &width, &height, GL_CLAMP_TO_EDGE, GL_LINEAR);
        // Now store menu resource for later use
        MenuResource menuResource = {textureID, width, height};
        resources.insert(std::pair<std::string, MenuResource>(elementName, menuResource));
    }
    jsonFile.close();

    return resources;
}

void MenuRenderer::Render() {
    m_projectionMatrix = glm::ortho(0.0f, (float) Config::get().resX, 0.0f, (float) Config::get().resY);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // TODO: Traverse parsed menuLayout JSON to render UI elements, depth sorted by layer back to front
    RenderText("OpenNFS v" + ONFS_VERSION + " Pre Alpha", 0, Config::get().resX - 270, 35, 0.2f, glm::vec3(0.6, 0.6, 0.6));
    RenderResource("onfsLogo", 0, Config::get().resX - 75, 5, 0.1f);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void MenuRenderer::RenderText(const std::string &text, GLint layer, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 colour) {
    ASSERT(layer >= 0 && layer <= 200, "Layer: " << layer << " is outside of range 0-200");
    // Allow for hot reload of shader
    m_fontShader.HotReload();

    // Activate corresponding render state
    m_fontShader.use();
    m_fontShader.loadLayer(layer);
    m_fontShader.loadColour(colour);
    m_fontShader.loadProjectionMatrix(m_projectionMatrix);

    glBindVertexArray(m_fontQuadVAO);
    // Iterate through all characters
    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
        Character ch = m_characterMap[*c];

        GLfloat xpos = x + ch.bearing.x * scale;
        GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

        GLfloat w = ch.size.x * scale;
        GLfloat h = ch.size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},     {xpos + w, ypos, 1.0, 1.0},

                                  {xpos, ypos + h, 0.0, 0.0}, {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};
        // Render glyph texture over quad
        m_fontShader.loadGlyphTexture(ch.textureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_fontQuadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_fontShader.unbind();
}

void MenuRenderer::RenderResource(const std::string &resourceID, GLint layer, GLfloat x, GLfloat y, GLfloat scale) {
    ASSERT(m_menuResourceMap.count(resourceID) > 0, "Requested resourceID " << resourceID << " not present in menu resource map");

    // TODO: Actually implement this rescaling scaling properly
    float ratioX = 1.0f; // (float) m_menuResourceMap[resourceID].width / Config::get().resX;
    float ratioY = 1.0f; // (float) m_menuResourceMap[resourceID].height / Config::get().resY;

    RenderResource(resourceID, layer, x, y, ratioX * m_menuResourceMap[resourceID].width, ratioY * m_menuResourceMap[resourceID].height, scale);
}

void MenuRenderer::RenderResource(const std::string &resourceID, GLint layer, GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat scale) {
    ASSERT(layer >= 0 && layer <= 200, "Layer: " << layer << " is outside of range 0-200");
    ASSERT(m_menuResourceMap.count(resourceID) > 0, "Requested resourceID " << resourceID << " not present in menu resource map");

    GLfloat xpos = x;
    GLfloat ypos = y;
    // Iterate through all characters
    GLfloat w = width * scale;
    GLfloat h = height * scale;
    // Update VBO
    GLfloat vertices[6][4] = {{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},     {xpos + w, ypos, 1.0, 1.0},

                              {xpos, ypos + h, 0.0, 0.0}, {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};

    // Allow for hot reload of shader
    m_menuShader.HotReload();

    // Activate corresponding render state
    m_menuShader.use();
    m_menuShader.loadLayer(layer);
    m_menuShader.loadColour(glm::vec3(1, 1, 1));
    m_menuShader.loadProjectionMatrix(m_projectionMatrix);
    // Render menu texture over quad
    m_menuShader.loadMenuTexture(m_menuResourceMap[resourceID].textureID);

    glBindVertexArray(m_menuQuadVAO);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, m_menuQuadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Reset state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_menuShader.unbind();
}
