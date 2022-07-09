#include "DepthShader.h"

const std::string vertexSrc = "../shaders/DepthVertexShader.vert";
const std::string fragSrc   = "../shaders/DepthFragmentShader.frag";

DepthShader::DepthShader() : super(vertexSrc, fragSrc) {
    bindAttributes();
    getAllUniformLocations();
}

void DepthShader::bindAttributes() {
    bindAttribute(0, "vertexPosition_modelspace");
    bindAttribute(1, "vertexUV");
    bindAttribute(2, "normal");
}

void DepthShader::getAllUniformLocations() {
    lightSpaceMatrixLocation     = getUniformLocation("lightSpaceMatrix");
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    textureArrayLocation         = getUniformLocation("textureArray");
}

void DepthShader::customCleanup() {
}

void DepthShader::bindTextureArray(GLuint textureArrayID) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
    glUniform1i(textureArrayLocation, 0);
}

void DepthShader::loadLightSpaceMatrix(const glm::mat4 &lightSpaceMatrix) {
    loadMat4(lightSpaceMatrixLocation, &lightSpaceMatrix[0][0]);
}

void DepthShader::loadTransformMatrix(const glm::mat4 &transformationMatrix) {
    loadMat4(transformationMatrixLocation, &transformationMatrix[0][0]);
}