#include "DepthShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/DepthShader.vert";
    std::string const fragSrc = "../shaders/DepthShader.frag";

    DepthShader::DepthShader() : BaseShader(vertexSrc, fragSrc) {
        DepthShader::bindAttributes();
        DepthShader::getAllUniformLocations();
    }

    void DepthShader::bindAttributes() {
        bindAttribute(0, "vertexPosition_modelspace");
        bindAttribute(1, "vertexUV");
        bindAttribute(2, "normal");
    }

    void DepthShader::getAllUniformLocations() {
        lightSpaceMatrixLocation = getUniformLocation("lightSpaceMatrix");
        transformationMatrixLocation = getUniformLocation("transformationMatrix");
        textureArrayLocation = getUniformLocation("textureArray");
    }

    void DepthShader::customCleanup() {
    }

    void DepthShader::bindTextureArray(GLuint const textureArrayID) const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
        glUniform1i(textureArrayLocation, 0);
    }

    void DepthShader::loadLightSpaceMatrix(glm::mat4 const &lightSpaceMatrix) {
        loadMat4(lightSpaceMatrixLocation, &lightSpaceMatrix[0][0]);
    }

    void DepthShader::loadTransformMatrix(glm::mat4 const &transformationMatrix) {
        loadMat4(transformationMatrixLocation, &transformationMatrix[0][0]);
    }
} // namespace OpenNFS
