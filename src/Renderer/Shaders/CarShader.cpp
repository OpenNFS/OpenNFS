#include "CarShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/CarShader.vert";
    std::string const fragSrc = "../shaders/CarShader.frag";

    CarShader::CarShader() : BaseShader(vertexSrc, fragSrc) {
        CarShader::bindAttributes();
        CarShader::getAllUniformLocations();
        loadEnvMapTextureData();
    }

    void CarShader::loadCarColour(glm::vec4 const colour, glm::vec4 const colour_secondary) const {
        loadVec4(colourLocation, colour);
        loadVec4(colourSecondaryLocation, colour_secondary);
    }

    void CarShader::loadCarTexture(GLuint const textureID) const {
        loadSampler2D(carTextureLocation, 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    void CarShader::loadLights(std::vector<LibOpenNFS::BaseLight const *> const &lights) const {
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

    void CarShader::loadSpecular(float const damper, float const reflectivity, float const env_reflectivity) const {
        loadFloat(shineDamperLocation, damper);
        loadFloat(reflectivityLocation, reflectivity);
        loadFloat(envReflectivityLocation, env_reflectivity);
    }

    void CarShader::loadProjectionViewMatrices(glm::mat4 const &projection, glm::mat4 const &view) const {
        loadMat4(viewMatrixLocation, &view[0][0]);
        loadMat4(projectionMatrixLocation, &projection[0][0]);
    }

    void CarShader::loadTransformationMatrix(glm::mat4 const &transformation) const {
        loadMat4(transformationMatrixLocation, &transformation[0][0]);
    }

    void CarShader::bindTextureArray(GLuint const textureArrayID) const {
        loadSampler2D(carTextureArrayLocation, 0);
        loadSampler2D(carTextureLocation, 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
    }

    void CarShader::setMultiTextured(bool const multiTextured) const {
        loadUint(isMultiTexturedLocation, multiTextured);
    }

    void CarShader::setPolyFlagged(bool const polyFlagged) const {
        loadUint(hasPolyFlagsLocation, polyFlagged);
    }

    void CarShader::loadEnvironmentMapTexture() const {
        loadSampler2D(envMapTextureLocation, 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, envMapTextureID);
    }

    void CarShader::loadCascadeData(CascadeData const &cascadeData) const {
        for (int i = 0; i < CSM_NUM_CASCADES; ++i) {
            loadMat4(lightSpaceMatricesLocation[i], &cascadeData.lightSpaceMatrices[i][0][0]);
        }
        glUniform1fv(cascadePlaneDistancesLocation, CSM_NUM_CASCADES, cascadeData.cascadePlaneDistances.data());
    }

    void CarShader::loadShadowMapTextureArray(GLuint const shadowMapTextureArrayID) const {
        loadSampler2D(shadowMapArrayLocation, 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapTextureArrayID);
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
        projectionMatrixLocation = getUniformLocation("projectionMatrix");
        viewMatrixLocation = getUniformLocation("viewMatrix");
        envMapTextureLocation = getUniformLocation("envMapTextureSampler");
        carTextureLocation = getUniformLocation("carTextureSampler");
        colourLocation = getUniformLocation("carColour");
        colourSecondaryLocation = getUniformLocation("carSecondaryColour");

        for (int i = 0; i < MAX_CAR_CONTRIB_LIGHTS; ++i) {
            lightPositionLocation[i] = getUniformLocation("lightPosition[" + std::to_string(i) + "]");
            lightColourLocation[i] = getUniformLocation("lightColour[" + std::to_string(i) + "]");
            attenuationLocation[i] = getUniformLocation("attenuation[" + std::to_string(i) + "]");
        }

        shineDamperLocation = getUniformLocation("shineDamper");
        reflectivityLocation = getUniformLocation("reflectivity");
        envReflectivityLocation = getUniformLocation("envReflectivity");
        carTextureArrayLocation = getUniformLocation("textureArray");
        isMultiTexturedLocation = getUniformLocation("multiTextured");
        hasPolyFlagsLocation = getUniformLocation("polyFlagged");

        // CSM uniforms
        for (int i = 0; i < CSM_NUM_CASCADES; ++i) {
            lightSpaceMatricesLocation[i] = getUniformLocation("lightSpaceMatrices[" + std::to_string(i) + "]");
        }
        cascadePlaneDistancesLocation = getUniformLocation("cascadePlaneDistances");
        shadowMapArrayLocation = getUniformLocation("shadowMapArray");
    }

    void CarShader::customCleanup() {
        glDeleteTextures(1, &envMapTextureID);
    }

    void CarShader::loadEnvMapTextureData() {
        std::string const filename{"../resources/misc/sky_textures/CHRD.BMP"};
        int width, height;
        envMapTextureID = ImageLoader::LoadImage(filename, &width, &height, GL_CLAMP_TO_EDGE, GL_LINEAR);
    }
} // namespace OpenNFS
