#include "UITextShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/UITextShader.vert";
    std::string const fragSrc = "../shaders/UITextShader.frag";

    UITextShader::UITextShader() : BaseShader(vertexSrc, fragSrc) {
        UITextShader::bindAttributes();
        UITextShader::getAllUniformLocations();
    }

    void UITextShader::bindAttributes() {
        bindAttribute(0, "vertexPosition_modelspace");
    }

    void UITextShader::getAllUniformLocations() {
        // Get handles for uniforms
        projectionMatrixLocation = getUniformLocation("projectionMatrix");
        layerLocation = getUniformLocation("layer");
        colourLocation = getUniformLocation("textColour");
        textGlyphSamplerLocation = getUniformLocation("textGlyphSampler");
    }

    void UITextShader::loadProjectionMatrix(glm::mat4 const &projection) const {
        loadMat4(projectionMatrixLocation, &projection[0][0]);
    }

    void UITextShader::loadLayer(GLint const layer, bool const isButtonText) const {
        float const layerZ = layer == 0 ? -0.999f : static_cast<float>(layer - 100) / 100;
        loadFloat(layerLocation, layerZ - 0.01);
    }

    void UITextShader::loadColour(glm::vec4 const colour) const {
        loadVec4(colourLocation, colour);
    }

    void UITextShader::loadGlyphTexture(GLuint const textureID) const {
        loadSampler2D(textGlyphSamplerLocation, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    void UITextShader::customCleanup() {
    }
} // namespace OpenNFS
