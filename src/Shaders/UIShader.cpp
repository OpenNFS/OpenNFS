#include "UIShader.h"

namespace OpenNFS {
    const std::string vertexSrc = "../shaders/UIVertexShader.vert";
    const std::string fragSrc = "../shaders/UIFragmentShader.frag";

    UIShader::UIShader() : BaseShader(vertexSrc, fragSrc) {
        UIShader::bindAttributes();
        UIShader::getAllUniformLocations();
    }

    void UIShader::bindAttributes() {
        bindAttribute(0, "vertexPosition_modelspace");
    }

    void UIShader::getAllUniformLocations() {
        // Get handles for uniforms
        projectionMatrixLocation = getUniformLocation("projectionMatrix");
        layerLocation = getUniformLocation("layer");
        colourLocation = getUniformLocation("colour");
        menuTextureSamplerLocation = getUniformLocation("menuTextureSampler");
    }

    void UIShader::loadProjectionMatrix(const glm::mat4 &projection) {
        loadMat4(projectionMatrixLocation, &projection[0][0]);
    }

    void UIShader::loadLayer(GLint layer) {
        loadFloat(layerLocation, layer == 0 ? -0.999f : (float) (layer - 100) / 100);
    }

    void UIShader::loadColour(glm::vec3 colour) {
        loadVec3(colourLocation, colour);
    }

    void UIShader::loadUITexture(GLuint textureID) {
        loadSampler2D(menuTextureSamplerLocation, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    void UIShader::customCleanup() {
    }
}
