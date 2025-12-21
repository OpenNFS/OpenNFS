#include "UITextShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/UITextShader.vert";
    std::string const fragSrc = "../shaders/UITextShader.frag";

    UITextShader::UITextShader() : BaseShader(vertexSrc, fragSrc) {
        UITextShader::bindAttributes();
        UITextShader::getAllUniformLocations();

        // Create the vertex buffer object
        glGenBuffers(1, &vbo);
    }

    void UITextShader::bindAttributes() {
        bindAttribute(0, "attribute_coord");
    }

    void UITextShader::getAllUniformLocations() {
        // Get handles for uniforms
        // projectionMatrixLocation = getUniformLocation("projectionMatrix");
        layerLocation = getUniformLocation("layer");
        colourLocation = getUniformLocation("color");
        textureLocation = getUniformLocation("tex");
    }

    void UITextShader::loadProjectionMatrix(glm::mat4 const &projection) const {
        loadMat4(projectionMatrixLocation, &projection[0][0]);
    }

    void UITextShader::loadLayer(GLint const layer) const {
        loadFloat(layerLocation, layer == 0 ? -0.999f : (float)(layer - 100) / 100);
    }

    void UITextShader::loadColour(glm::vec4 const colour) const {
        loadVec4(colourLocation, colour);
    }

    void UITextShader::loadUITexture(GLuint const textureID) const {
        loadUint(textureLocation, textureID);
    }

    void UITextShader::bindBuffer() const {
        glEnableVertexAttribArray(attribute_coord);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    void UITextShader::unbindBuffer() const {
        glDisableVertexAttribArray(attribute_coord);
    }

    void UITextShader::customCleanup() {
    }
} // namespace OpenNFS
