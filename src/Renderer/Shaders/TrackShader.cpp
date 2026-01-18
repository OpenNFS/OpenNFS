#include "TrackShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/TrackShader.vert";
    std::string const fragSrc = "../shaders/TrackShader.frag";

    TrackShader::TrackShader() : BaseShader(vertexSrc, fragSrc) {
        TrackShader::bindAttributes();
        TrackShader::getAllUniformLocations();
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
        projectionMatrixLocation = getUniformLocation("projectionMatrix");
        viewMatrixLocation = getUniformLocation("viewMatrix");
        lightSpaceMatrixLocation = getUniformLocation("lightSpaceMatrix");
        trackTextureArrayLocation = getUniformLocation("textureArray");
        shineDamperLocation = getUniformLocation("shineDamper");
        reflectivityLocation = getUniformLocation("reflectivity");
        useClassicLocation = getUniformLocation("useClassic");
        shadowMapTextureLocation = getUniformLocation("shadowMap");
        shadowMapArrayLocation = getUniformLocation("shadowMapArray");
        ambientFactorLocation = getUniformLocation("ambientFactor");

        for (int i = 0; i < MAX_TRACK_CONTRIB_LIGHTS; ++i) {
            lightPositionLocation[i] = getUniformLocation("lightPosition[" + std::to_string(i) + "]");
            lightColourLocation[i] = getUniformLocation("lightColour[" + std::to_string(i) + "]");
            attenuationLocation[i] = getUniformLocation("attenuation[" + std::to_string(i) + "]");
        }
        for (int i = 0; i < MAX_SPOTLIGHTS; ++i) {
            spotlightPositionLocation[i] = getUniformLocation("spotlightPosition[" + std::to_string(i) + "]");
            spotlightColourLocation[i] = getUniformLocation("spotlightColour[" + std::to_string(i) + "]");
            spotlightDirectionLocation[i] = getUniformLocation("spotlightDirection[" + std::to_string(i) + "]");
            spotlightInnerCutOffLocation[i] = getUniformLocation("spotlightInnerCutOff[" + std::to_string(i) + "]");
            spotlightOuterCutOffLocation[i] = getUniformLocation("spotlightOuterCutOff[" + std::to_string(i) + "]");
        }

        // CSM uniforms
        for (int i = 0; i < CSM_NUM_CASCADES; ++i) {
            lightSpaceMatricesLocation[i] = getUniformLocation("lightSpaceMatrices[" + std::to_string(i) + "]");
        }
        cascadePlaneDistancesLocation = getUniformLocation("cascadePlaneDistances");
    }

    void TrackShader::customCleanup() {
    }

    void TrackShader::bindTextureArray(GLuint const textureArrayID) const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
        glUniform1i(trackTextureArrayLocation, 0);
    }

    void TrackShader::loadLights(std::vector<LibOpenNFS::BaseLight const *> const &lights) const {
        for (int i = 0; i < MAX_TRACK_CONTRIB_LIGHTS; ++i) {
            if (i < lights.size() && lights[i]->active) {
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

    void TrackShader::loadSpotlights(std::vector<Spotlight> const &spotlights) const {
        for (int i = 0; i < MAX_SPOTLIGHTS; ++i) {
            if (i < spotlights.size()) {
                loadVec3(spotlightPositionLocation[i], spotlights[i].position);
                loadVec3(spotlightColourLocation[i], spotlights[i].colour);
                loadVec3(spotlightDirectionLocation[i], spotlights[i].direction);
                loadFloat(spotlightInnerCutOffLocation[i], spotlights[i].innerCutOff);
                loadFloat(spotlightOuterCutOffLocation[i], spotlights[i].outerCutOff);
            } else {
                // Set inactive spotlights to zero
                loadVec3(spotlightPositionLocation[i], glm::vec3(0, 0, 0));
                loadVec3(spotlightColourLocation[i], glm::vec3(0, 0, 0));
                loadVec3(spotlightDirectionLocation[i], glm::vec3(0, 0, 0));
                loadFloat(spotlightInnerCutOffLocation[i], 0.0f);
                loadFloat(spotlightOuterCutOffLocation[i], 0.0f);
            }
        }
    }

    void TrackShader::setClassic(bool const useClassic) const {
        loadUint(useClassicLocation, useClassic);
    }

    void TrackShader::loadSpecular(float const damper, float const reflectivity) const {
        loadFloat(shineDamperLocation, damper);
        loadFloat(reflectivityLocation, reflectivity);
    }

    void TrackShader::loadProjectionViewMatrices(glm::mat4 const &projection, glm::mat4 const &view) const {
        loadMat4(viewMatrixLocation, &view[0][0]);
        loadMat4(projectionMatrixLocation, &projection[0][0]);
    }

    void TrackShader::loadTransformMatrix(glm::mat4 const &transformation) const {
        loadMat4(transformationMatrixLocation, &transformation[0][0]);
    }

    void TrackShader::loadLightSpaceMatrix(glm::mat4 const &lightSpaceMatrix) const {
        loadMat4(lightSpaceMatrixLocation, &lightSpaceMatrix[0][0]);
    }

    void TrackShader::loadShadowMapTexture(GLuint const shadowMapTextureID) const {
        loadSampler2D(shadowMapTextureLocation, 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowMapTextureID);
    }

    void TrackShader::loadShadowMapTextureArray(GLuint const shadowMapTextureArrayID) const {
        loadSampler2D(shadowMapArrayLocation, 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapTextureArrayID);
    }

    void TrackShader::loadCascadeData(CascadeData const &cascadeData) const {
        for (int i = 0; i < CSM_NUM_CASCADES; ++i) {
            loadMat4(lightSpaceMatricesLocation[i], &cascadeData.lightSpaceMatrices[i][0][0]);
        }
        glUniform1fv(cascadePlaneDistancesLocation, CSM_NUM_CASCADES, cascadeData.cascadePlaneDistances.data());
    }

    void TrackShader::loadAmbientFactor(float const ambientFactor) const {
        loadFloat(ambientFactorLocation, ambientFactor);
    }

} // namespace OpenNFS
