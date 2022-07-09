#include "SkydomeShader.h"

const std::string vertexSrc = "../shaders/SkydomeVertexShader.vert";
const std::string fragSrc   = "../shaders/SkydomeFragmentShader.frag";

SkydomeShader::SkydomeShader() : super(vertexSrc, fragSrc) {
    bindAttributes();
    getAllUniformLocations();
}

void SkydomeShader::bindAttributes() {
    bindAttribute(0, "vertexPosition_modelspace");
    bindAttribute(1, "vertexUV");
    bindAttribute(2, "normal");
}

void SkydomeShader::getAllUniformLocations() {
    // Vertex Shader Uniforms
    sunPositionLocation          = getUniformLocation("sunPosition");
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    projectionMatrixLocation     = getUniformLocation("projectionMatrix");
    viewMatrixLocation           = getUniformLocation("viewMatrix");
    starRotationMatrixLocation   = getUniformLocation("starRotationMatrix");

    // Fragment Shader Uniform
    tintTextureLocation    = getUniformLocation("tint");
    tint2TextureLocation   = getUniformLocation("tint2");
    sunTextureLocation     = getUniformLocation("sun");
    moonTextureLocation    = getUniformLocation("moon");
    clouds1TextureLocation = getUniformLocation("clouds1");
    clouds2TextureLocation = getUniformLocation("clouds2");
    weatherLocation        = getUniformLocation("weather");
    timeLocation           = getUniformLocation("time");
}

void SkydomeShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation) {
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}

void SkydomeShader::loadStarRotationMatrix(const glm::mat3 &star_rotation_matrix) {
    loadMat3(starRotationMatrixLocation, &star_rotation_matrix[0][0]);
}

void SkydomeShader::loadSunPosition(const std::shared_ptr<GlobalLight> &light) {
    loadVec3(sunPositionLocation, light->position);
}

void SkydomeShader::loadTextures(GLuint clouds1TextureID, GLuint clouds2TextureID, GLuint sunTextureID, GLuint moonTextureID, GLuint tintTextureID, GLuint tint2TextureID) {
    loadSampler2D(clouds1TextureLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, clouds1TextureID);

    loadSampler2D(clouds2TextureLocation, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, clouds2TextureID);

    loadSampler2D(sunTextureLocation, 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, sunTextureID);

    loadSampler2D(moonTextureLocation, 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, moonTextureID);

    loadSampler2D(tintTextureLocation, 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, tintTextureID);

    loadSampler2D(tint2TextureLocation, 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, tint2TextureID);
}

void SkydomeShader::loadWeatherMixFactor(float weatherMixFactor) {
    loadFloat(weatherLocation, weatherMixFactor);
}

void SkydomeShader::loadTime(float time) {
    loadFloat(timeLocation, time);
}

void SkydomeShader::customCleanup() {
}
