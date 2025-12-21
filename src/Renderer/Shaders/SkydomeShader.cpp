#include "SkydomeShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/SkydomeShader.vert";
    std::string const fragSrc = "../shaders/SkydomeShader.frag";

    SkydomeShader::SkydomeShader() : BaseShader(vertexSrc, fragSrc) {
        SkydomeShader::bindAttributes();
        SkydomeShader::getAllUniformLocations();
    }

    void SkydomeShader::bindAttributes() {
        bindAttribute(0, "vertexPosition_modelspace");
        bindAttribute(1, "vertexUV");
        bindAttribute(2, "normal");
    }

    void SkydomeShader::getAllUniformLocations() {
        // Vertex Shader Uniforms
        sunPositionLocation = getUniformLocation("sunPosition");
        transformationMatrixLocation = getUniformLocation("transformationMatrix");
        projectionMatrixLocation = getUniformLocation("projectionMatrix");
        viewMatrixLocation = getUniformLocation("viewMatrix");
        starRotationMatrixLocation = getUniformLocation("starRotationMatrix");

        // Fragment Shader Uniform
        tintTextureLocation = getUniformLocation("tint");
        tint2TextureLocation = getUniformLocation("tint2");
        sunTextureLocation = getUniformLocation("sun");
        moonTextureLocation = getUniformLocation("moon");
        clouds1TextureLocation = getUniformLocation("clouds1");
        clouds2TextureLocation = getUniformLocation("clouds2");
        weatherLocation = getUniformLocation("weather");
        timeLocation = getUniformLocation("time");
    }

    void SkydomeShader::loadMatrices(glm::mat4 const &projection, glm::mat4 const &view, glm::mat4 const &transformation) const {
        loadMat4(viewMatrixLocation, &view[0][0]);
        loadMat4(projectionMatrixLocation, &projection[0][0]);
        loadMat4(transformationMatrixLocation, &transformation[0][0]);
    }

    void SkydomeShader::loadStarRotationMatrix(glm::mat3 const &star_rotation_matrix) const {
        loadMat3(starRotationMatrixLocation, &star_rotation_matrix[0][0]);
    }

    void SkydomeShader::loadSunPosition(GlobalLight const *light) const {
        loadVec3(sunPositionLocation, light->position);
    }

    void SkydomeShader::loadTextures(GLuint const clouds1TextureID, GLuint const clouds2TextureID, GLuint const sunTextureID,
                                     GLuint const moonTextureID, GLuint const tintTextureID, GLuint const tint2TextureID) const {
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

    void SkydomeShader::loadWeatherMixFactor(float const weatherMixFactor) const {
        loadFloat(weatherLocation, weatherMixFactor);
    }

    void SkydomeShader::loadTime(float const time) const {
        loadFloat(timeLocation, time);
    }

    void SkydomeShader::customCleanup() {
    }
} // namespace OpenNFS
