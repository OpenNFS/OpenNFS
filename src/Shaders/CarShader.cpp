#include "CarShader.h"

const std::string vertexSrc = "../shaders/CarVertexShader.vert";
const std::string fragSrc   = "../shaders/CarFragmentShader.frag";

CarShader::CarShader() : super(vertexSrc, fragSrc) {
    bindAttributes();
    getAllUniformLocations();
    loadEnvMapTextureData();
}

void CarShader::loadEnvMapTextureData() {
    std::string filename = "../resources/misc/sky_textures/CHRD.BMP";
    int width, height;
    envMapTextureID = ImageLoader::LoadImage(filename, &width, &height, GL_CLAMP_TO_EDGE, GL_LINEAR);
}

void CarShader::bindAttributes() {
    bindAttribute(0, "vertexPosition_modelspace");
    bindAttribute(1, "vertexUV");
    bindAttribute(2, "normal");
    bindAttribute(3, "textureIndex");
    bindAttribute(4, "polygonFlag");
}

void CarShader::getAllUniformLocations() {
    // Get handles for uniforms
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    projectionMatrixLocation     = getUniformLocation("projectionMatrix");
    viewMatrixLocation           = getUniformLocation("viewMatrix");
    envMapTextureLocation        = getUniformLocation("envMapTextureSampler");
    carTextureLocation           = getUniformLocation("carTextureSampler");
    colourLocation               = getUniformLocation("carColour");

    for (int i = 0; i < MAX_CAR_CONTRIB_LIGHTS; ++i) {
        lightPositionLocation[i] = getUniformLocation("lightPosition[" + std::to_string(i) + "]");
        lightColourLocation[i]   = getUniformLocation("lightColour[" + std::to_string(i) + "]");
        attenuationLocation[i]   = getUniformLocation("attenuation[" + std::to_string(i) + "]");
    }

    shineDamperLocation     = getUniformLocation("shineDamper");
    reflectivityLocation    = getUniformLocation("reflectivity");
    envReflectivityLocation = getUniformLocation("envReflectivity");
    carTextureArrayLocation = getUniformLocation("textureArray");
    isMultiTexturedLocation = getUniformLocation("multiTextured");
    hasPolyFlagsLocation    = getUniformLocation("polyFlagged");
}

void CarShader::setMultiTextured(bool multiTextured) {
    loadBool(isMultiTexturedLocation, multiTextured);
}

void CarShader::setPolyFlagged(bool polyFlagged) {
    loadBool(hasPolyFlagsLocation, polyFlagged);
}

void CarShader::customCleanup() {
    glDeleteTextures(1, &envMapTextureID);
}

void CarShader::bindTextureArray(GLuint textureArrayID) {
    loadSampler2D(carTextureArrayLocation, 0);
    loadSampler2D(carTextureLocation, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
}

void CarShader::loadCarTexture(GLuint textureID) {
    loadSampler2D(carTextureLocation, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void CarShader::loadEnvironmentMapTexture() {
    loadSampler2D(envMapTextureLocation, 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, envMapTextureID);
}

void CarShader::loadSpecular(float damper, float reflectivity, float env_reflectivity) {
    loadFloat(shineDamperLocation, damper);
    loadFloat(reflectivityLocation, reflectivity);
    loadFloat(envReflectivityLocation, env_reflectivity);
}

void CarShader::loadProjectionViewMatrices(const glm::mat4 &projection, const glm::mat4 &view) {
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
}

void CarShader::loadTransformationMatrix(const glm::mat4 &transformation) {
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}

void CarShader::loadLights(const std::vector<shared_ptr<BaseLight>> &lights) {
    for (int i = 0; i < MAX_CAR_CONTRIB_LIGHTS; ++i) {
        if (i < lights.size()) {
            loadVec3(lightPositionLocation[i], lights[i]->position);
            loadVec4(lightColourLocation[i], lights[i]->colour);
            loadVec3(attenuationLocation[i], lights[i]->attenuation);
        } else {
            loadVec3(lightPositionLocation[i], glm::vec3(0, 0, 0));
            loadVec4(lightColourLocation[i], glm::vec4(0, 0, 0, 0));
            loadVec3(attenuationLocation[i], glm::vec3(1, 0, 0));
        }
    }
}

void CarShader::loadCarColor(glm::vec3 color) {
    loadVec3(colourLocation, color);
}
