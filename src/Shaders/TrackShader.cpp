#include "TrackShader.h"

const std::string vertexSrc = "../shaders/TrackVertexShader.vert";
const std::string fragSrc   = "../shaders/TrackFragmentShader.frag";

TrackShader::TrackShader() : super(vertexSrc, fragSrc) {
    bindAttributes();
    getAllUniformLocations();
}

void TrackShader::bindAttributes() {
    bindAttribute(0, "vertexPosition_modelspace");
    bindAttribute(1, "vertexUV");
    bindAttribute(2, "textureIndex");
    bindAttribute(3, "nfsData");
    bindAttribute(4, "normal");
    bindAttribute(5, "debugData");
}

void TrackShader::getAllUniformLocations() {
    // Get handles for uniforms
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    projectionMatrixLocation     = getUniformLocation("projectionMatrix");
    viewMatrixLocation           = getUniformLocation("viewMatrix");
    lightSpaceMatrixLocation     = getUniformLocation("lightSpaceMatrix");
    trackTextureArrayLocation    = getUniformLocation("textureArray");
    shineDamperLocation          = getUniformLocation("shineDamper");
    reflectivityLocation         = getUniformLocation("reflectivity");
    useClassicLocation           = getUniformLocation("useClassic");
    shadowMapTextureLocation     = getUniformLocation("shadowMap");
    ambientFactorLocation        = getUniformLocation("ambientFactor");

    for (int i = 0; i < MAX_TRACK_CONTRIB_LIGHTS; ++i) {
        lightPositionLocation[i] = getUniformLocation("lightPosition[" + std::to_string(i) + "]");
        lightColourLocation[i]   = getUniformLocation("lightColour[" + std::to_string(i) + "]");
        attenuationLocation[i]   = getUniformLocation("attenuation[" + std::to_string(i) + "]");
    }
    spotlightPositionLocation  = getUniformLocation("spotlightPosition");
    spotlightColourLocation    = getUniformLocation("spotlightColour");
    spotlightDirectionLocation = getUniformLocation("spotlightDirection");
    spotlightCutOffLocation    = getUniformLocation("spotlightCutOff");
}

void TrackShader::customCleanup() {
}

void TrackShader::bindTextureArray(GLuint textureArrayID) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
    glUniform1i(trackTextureArrayLocation, 0);
}

void TrackShader::loadLights(const std::vector<shared_ptr<BaseLight>> &lights) {
    for (int i = 0; i < MAX_TRACK_CONTRIB_LIGHTS; ++i) {
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

void TrackShader::loadSpotlight(Spotlight spotlight) {
    loadVec3(spotlightPositionLocation, spotlight.position);
    loadVec3(spotlightColourLocation, spotlight.colour);
    loadVec3(spotlightDirectionLocation, spotlight.direction);
    loadFloat(spotlightCutOffLocation, spotlight.cutOff);
}

void TrackShader::setClassic(bool useClassic) {
    loadBool(useClassicLocation, useClassic);
}

void TrackShader::loadSpecular(float damper, float reflectivity) {
    loadFloat(shineDamperLocation, damper);
    loadFloat(reflectivityLocation, reflectivity);
}

void TrackShader::loadProjectionViewMatrices(const glm::mat4 &projection, const glm::mat4 &view) {
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
}

void TrackShader::loadTransformMatrix(const glm::mat4 &transformation) {
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}

void TrackShader::loadLightSpaceMatrix(const glm::mat4 &lightSpaceMatrix) {
    loadMat4(lightSpaceMatrixLocation, &lightSpaceMatrix[0][0]);
}

void TrackShader::loadShadowMapTexture(GLuint shadowMapTextureID) {
    loadSampler2D(shadowMapTextureLocation, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMapTextureID);
}

void TrackShader::loadAmbientFactor(float ambientFactor) {
    loadFloat(ambientFactorLocation, ambientFactor);
}
