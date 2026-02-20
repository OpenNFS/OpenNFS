#include "UIShapeShader.h"

namespace OpenNFS {
    std::string const shapeVertSrc = "../shaders/UIShapeShader.vert";
    std::string const shapeFragSrc = "../shaders/UIShapeShader.frag";

    UIShapeShader::UIShapeShader() : BaseShader(shapeVertSrc, shapeFragSrc) {
        UIShapeShader::bindAttributes();
        UIShapeShader::getAllUniformLocations();
    }

    void UIShapeShader::bindAttributes() {
        bindAttribute(0, "vertexPosition_modelspace");
    }

    void UIShapeShader::getAllUniformLocations() {
        projectionMatrixLocation = getUniformLocation("projectionMatrix");
        layerLocation            = getUniformLocation("layer");
        colourLocation           = getUniformLocation("colour");
    }

    void UIShapeShader::loadProjectionMatrix(glm::mat4 const &projection) const {
        loadMat4(projectionMatrixLocation, &projection[0][0]);
    }

    void UIShapeShader::loadLayer(GLint const layer) const {
        loadFloat(layerLocation, layer == 0 ? -0.999f : static_cast<float>(layer - 100) / 100.0f);
    }

    void UIShapeShader::loadColour(glm::vec4 const colour) const {
        loadVec4(colourLocation, colour);
    }

    void UIShapeShader::customCleanup() {
    }
} // namespace OpenNFS