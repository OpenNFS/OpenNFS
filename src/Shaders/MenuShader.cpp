#include "MenuShader.h"

const std::string vertexSrc = "../shaders/MenuVertexShader.vert";
const std::string fragSrc   = "../shaders/MenuFragmentShader.frag";

MenuShader::MenuShader() : super(vertexSrc, fragSrc) {
    bindAttributes();
    getAllUniformLocations();
}

void MenuShader::bindAttributes() {
    bindAttribute(0, "vertexPosition_modelspace");
}

void MenuShader::getAllUniformLocations() {
    // Get handles for uniforms
    projectionMatrixLocation   = getUniformLocation("projectionMatrix");
    layerLocation              = getUniformLocation("layer");
    colourLocation             = getUniformLocation("colour");
    menuTextureSamplerLocation = getUniformLocation("menuTextureSampler");
}

void MenuShader::loadProjectionMatrix(const glm::mat4 &projection) {
    loadMat4(projectionMatrixLocation, &projection[0][0]);
}

void MenuShader::loadLayer(GLint layer) {
    loadFloat(layerLocation, layer == 0 ? -0.999f : (float) (layer - 100) / 100);
}

void MenuShader::loadColour(glm::vec3 colour) {
    loadVec3(colourLocation, colour);
}

void MenuShader::loadMenuTexture(GLuint textureID) {
    loadSampler2D(menuTextureSamplerLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void MenuShader::customCleanup() {
}