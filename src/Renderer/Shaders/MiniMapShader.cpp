#include "MiniMapShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/MiniMapShader.vert";
    std::string const fragSrc = "../shaders/MiniMapShader.frag";

    MiniMapShader::MiniMapShader() : BaseShader(vertexSrc, fragSrc) {
        MiniMapShader::bindAttributes();
        MiniMapShader::getAllUniformLocations();
    }

    void MiniMapShader::bindAttributes() {
        bindAttribute(0, "vertexPosition_modelspace");
    }

    void MiniMapShader::getAllUniformLocations() {
        // Get handles for uniforms
        transformationMatrixLocation = getUniformLocation("transformationMatrix");
        projectionMatrixLocation = getUniformLocation("projectionMatrix");
        colourLocation = getUniformLocation("spriteColour");
        layerLocation = getUniformLocation("layer");
    }

    void MiniMapShader::loadProjectionMatrix(glm::mat4 const &projection) const {
        loadMat4(projectionMatrixLocation, &projection[0][0]);
    }

    void MiniMapShader::loadTransformationMatrix(glm::mat4 const &transformation) const {
        loadMat4(transformationMatrixLocation, &transformation[0][0]);
    }

    void MiniMapShader::loadColor(glm::vec3 const color) const {
        loadVec3(colourLocation, color);
    }

    void MiniMapShader::loadLayer(GLint const layer) const {
        loadFloat(layerLocation, layer == 0 ? -0.999f : static_cast<float>(layer - 100) / 100);
    }

    void MiniMapShader::customCleanup() {
    }
} // namespace OpenNFS
