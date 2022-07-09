#include "BulletShader.h"

const std::string vertexSrc = "../shaders/BulletVertexShader.vert";
const std::string fragSrc   = "../shaders/BulletFragmentShader.frag";

BulletShader::BulletShader() : BaseShader(vertexSrc, fragSrc) {
    bindAttributes();
    getAllUniformLocations();
}

void BulletShader::bindAttributes() {
    bindAttribute(0, "vertexPosition_modelspace");
    bindAttribute(1, "vertexColour");
}

void BulletShader::getAllUniformLocations() {
    // Get handles for uniforms
    projectionViewMatrixLocation = getUniformLocation("projectionViewMatrix");
}

void BulletShader::loadProjectionViewMatrix(const glm::mat4 &projectionViewMatrix) {
    loadMat4(projectionViewMatrixLocation, &projectionViewMatrix[0][0]);
}

void BulletShader::customCleanup() {
}