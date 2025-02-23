#include "MiniMapShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/MiniMapVertexShader.vert";
    std::string const fragSrc = "../shaders/MiniMapFragmentShader.frag";

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

    void MiniMapShader::loadProjectionMatrix(glm::mat4 const &projection) {
        loadMat4(projectionMatrixLocation, &projection[0][0]);
    }

    void MiniMapShader::loadTransformationMatrix(glm::mat4 const &transformation) {
        loadMat4(transformationMatrixLocation, &transformation[0][0]);
    }

    void MiniMapShader::loadColor(glm::vec3 const color) {
        loadVec3(colourLocation, color);
    }

    void MiniMapShader::loadLayer(const GLint layer) {
        loadFloat(layerLocation, layer == 0 ? -0.999f : static_cast<float>(layer - 100) / 100);
    }

    void MiniMapShader::customCleanup() {
    }
} // namespace OpenNFS
