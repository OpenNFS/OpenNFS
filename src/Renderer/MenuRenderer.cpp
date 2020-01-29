#include "MenuRenderer.h"

MenuRenderer::MenuRenderer()
{
    FT_Library ft;
    ASSERT(!FT_Init_FreeType(&ft), "FREETYPE: Could not init FreeType Library");

    FT_Face face;
    ASSERT(!FT_New_Face(ft, "../resources/ui/fonts/earth.ttf", 0, &face), "FREETYPE: Failed to load font");

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
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
        characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Configure VAO/VBO for font texture quads
    glGenVertexArrays(1, &fontQuadVAO);
    glGenBuffers(1, &fontQuadVBO);
    glBindVertexArray(fontQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fontQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    // Configure VAO/VBO for menu texture quads
    glGenVertexArrays(1, &menuQuadVAO);
    glGenBuffers(1, &menuQuadVBO);
    glBindVertexArray(menuQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, menuQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    // Reset state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    LOG(INFO) << "Glyphs loaded successfully";

    menuResources = loadResources("../resources/ui/menu/resources.json");
    LOG(INFO) << "Menu resources loaded successfully";
}

MenuRenderer::~MenuRenderer()
{
    // Lets delete all of the loaded textures
    for (std::map<GLchar, Character>::iterator it = characters.begin(); it != characters.end(); ++it)
    {
        glDeleteTextures(1, &it->second.textureID);
    }
    for (std::map<std::string, MenuResource>::iterator it = menuResources.begin(); it != menuResources.end(); ++it)
    {
        glDeleteTextures(1, &it->second.textureID);
    }
}

std::map<std::string, MenuResource> MenuRenderer::loadResources(const std::string &resourceFile)
{
    // Read the resource JSON file
    std::ifstream jsonFile(resourceFile);
    ASSERT(jsonFile.is_open(), "Couldn't open menu resource file " << resourceFile);

    std::map<std::string, MenuResource> resources;
    json resourcesJson;
    jsonFile >> resourcesJson;

    for (auto &el : resourcesJson["resources"].items())
    {
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

void MenuRenderer::render()
{
    projectionMatrix = glm::ortho(0.0f, (float) Config::get().resX, 0.0f, (float) Config::get().resY);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // TODO: Traverse parsed menuLayout JSON to render UI elements, depth sorted by layer back to front
    renderResource("backgroundPattern", 0, 0, 0, Config::get().resX, Config::get().resY, 1.0f);
    renderResource("leftMenuCurve", 1, 0, 0);
    renderText("Test Menu", 2, Config::get().resX / 2, Config::get().resY / 2, 1.0f, glm::vec3(0.9, 0.9, 0));
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void MenuRenderer::renderText(const std::string &text, GLint layer, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 colour)
{
    ASSERT(layer >= 0 && layer <= 200, "Layer: " << layer << " is outside of range 0-200");
    // Allow for hot reload of shader
    fontShader.shaders.UpdatePrograms();

    // Activate corresponding render state
    fontShader.use();
    fontShader.loadLayer(layer);
    fontShader.loadColour(colour);
    fontShader.loadProjectionMatrix(projectionMatrix);

    glBindVertexArray(fontQuadVAO);
    // Iterate through all characters
    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c)
    {
        Character ch = characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},     {xpos + w, ypos, 1.0, 1.0},

                                  {xpos, ypos + h, 0.0, 0.0}, {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};
        // Render glyph texture over quad
        fontShader.loadGlyphTexture(ch.textureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, fontQuadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    fontShader.unbind();
}

void MenuRenderer::renderResource(const std::string &resourceID, GLint layer, GLfloat x, GLfloat y)
{
    ASSERT(menuResources.count(resourceID) > 0, "Requested resourceID " << resourceID << " not present in menu resource map");

    float ratioX = (float) menuResources[resourceID].width / Config::get().resX;
    float ratioY = (float) menuResources[resourceID].height / Config::get().resY;

    // TODO: Actually implement this rescaling scaling properly

    renderResource(resourceID, layer, x, y, ratioX * menuResources[resourceID].width, ratioY * menuResources[resourceID].height, 1.0f);
}

void MenuRenderer::renderResource(const std::string &resourceID, GLint layer, GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat scale)
{
    ASSERT(layer >= 0 && layer <= 200, "Layer: " << layer << " is outside of range 0-200");
    ASSERT(menuResources.count(resourceID) > 0, "Requested resourceID " << resourceID << " not present in menu resource map");

    GLfloat xpos = x;
    GLfloat ypos = y;
    // Iterate through all characters
    GLfloat w = width * scale;
    GLfloat h = height * scale;
    // Update VBO
    GLfloat vertices[6][4] = {{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},     {xpos + w, ypos, 1.0, 1.0},

                              {xpos, ypos + h, 0.0, 0.0}, {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};

    // Allow for hot reload of shader
    menuShader.shaders.UpdatePrograms();

    // Activate corresponding render state
    menuShader.use();
    menuShader.loadLayer(layer);
    menuShader.loadColour(glm::vec3(1, 1, 1));
    menuShader.loadProjectionMatrix(projectionMatrix);
    // Render menu texture over quad
    menuShader.loadMenuTexture(menuResources[resourceID].textureID);

    glBindVertexArray(menuQuadVAO);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, menuQuadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Reset state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    menuShader.unbind();
}
