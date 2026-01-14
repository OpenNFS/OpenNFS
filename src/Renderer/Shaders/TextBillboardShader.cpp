#include "TextBillboardShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/TextBillboardShader.vert";
    std::string const fragSrc = "../shaders/TextBillboardShader.frag";

    TextBillboardShader::TextBillboardShader() : BaseShader(vertexSrc, fragSrc) {
        TextBillboardShader::bindAttributes();
        TextBillboardShader::getAllUniformLocations();
    }

    void TextBillboardShader::bindAttributes() {
        bindAttribute(0, "vertexPosition_modelspace");
    }

    void TextBillboardShader::getAllUniformLocations() {
        // Get handles for uniforms
        projectionMatrixLocation = getUniformLocation("projectionMatrix");
        viewMatrixLocation = getUniformLocation("viewMatrix");
        billboardPosLocation = getUniformLocation("billboardPos");
        scaleLocation = getUniformLocation("scale");
        colourLocation = getUniformLocation("textColour");
        textGlyphSamplerLocation = getUniformLocation("textGlyphSampler");
    }

    void TextBillboardShader::loadMatrices(glm::mat4 const &projection, glm::mat4 const &view) const {
        loadMat4(projectionMatrixLocation, &projection[0][0]);
        loadMat4(viewMatrixLocation, &view[0][0]);
    }

    void TextBillboardShader::loadBillboardPos(glm::vec3 const &pos) const {
        loadVec3(billboardPosLocation, pos);
    }

    void TextBillboardShader::loadScale(float const scale) const {
        loadFloat(scaleLocation, scale);
    }

    void TextBillboardShader::loadColour(glm::vec4 const &colour) const {
        loadVec4(colourLocation, colour);
    }

    void TextBillboardShader::loadGlyphTexture(GLuint const textureID) const {
        loadSampler2D(textGlyphSamplerLocation, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    void TextBillboardShader::customCleanup() {
    }
} // namespace OpenNFS