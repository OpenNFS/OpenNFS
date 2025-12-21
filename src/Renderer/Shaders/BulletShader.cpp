#include "BulletShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/BulletShader.vert";
    std::string const fragSrc = "../shaders/BulletShader.frag";

    BulletShader::BulletShader() : BaseShader(vertexSrc, fragSrc) {
        BulletShader::bindAttributes();
        BulletShader::getAllUniformLocations();
    }

    void BulletShader::bindAttributes() {
        bindAttribute(0, "vertexPosition_modelspace");
        bindAttribute(1, "vertexColour");
    }

    void BulletShader::getAllUniformLocations() {
        // Get handles for uniforms
        projectionViewMatrixLocation = getUniformLocation("projectionViewMatrix");
    }

    void BulletShader::loadProjectionViewMatrix(glm::mat4 const &projectionViewMatrix) const {
        loadMat4(projectionViewMatrixLocation, &projectionViewMatrix[0][0]);
    }

    void BulletShader::customCleanup() {
    }
} // namespace OpenNFS
