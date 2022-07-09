#include "RaceNetShader.h"

const std::string vertexSrc = "../shaders/RaceNetVertexShader.vert";
const std::string fragSrc   = "../shaders/RaceNetFragmentShader.frag";

RaceNetShader::RaceNetShader() : super(vertexSrc, fragSrc) {
    bindAttributes();
    getAllUniformLocations();
}

void RaceNetShader::bindAttributes() {
    bindAttribute(0, "vertexPosition_modelspace");
}

void RaceNetShader::getAllUniformLocations() {
    // Get handles for uniforms
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    projectionMatrixLocation     = getUniformLocation("projectionMatrix");
    colourLocation               = getUniformLocation("spriteColour");
}

void RaceNetShader::loadProjectionMatrix(const glm::mat4 &projection) {
    loadMat4(projectionMatrixLocation, &projection[0][0]);
}

void RaceNetShader::loadTransformationMatrix(const glm::mat4 &transformation) {
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}

void RaceNetShader::loadColor(glm::vec3 color) {
    loadVec3(colourLocation, color);
}

void RaceNetShader::customCleanup() {
}
