#include "SkidMarkShader.h"

namespace OpenNFS {
    std::string const skidVertexSrc = "../shaders/SkidMarkShader.vert";
    std::string const skidFragSrc = "../shaders/SkidMarkShader.frag";

    SkidMarkShader::SkidMarkShader() : BaseShader(skidVertexSrc, skidFragSrc) {
        SkidMarkShader::bindAttributes();
        SkidMarkShader::getAllUniformLocations();
    }

    void SkidMarkShader::bindAttributes() {
        bindAttribute(0, "vertexPosition");
        bindAttribute(1, "vertexUV");
        bindAttribute(2, "vertexNormal");
        bindAttribute(3, "vertexAge");
        bindAttribute(4, "vertexIntensity");
        bindAttribute(5, "vertexSurfaceType");
    }

    void SkidMarkShader::getAllUniformLocations() {
        m_projectionMatrixLocation = getUniformLocation("projectionMatrix");
        m_viewMatrixLocation = getUniformLocation("viewMatrix");
        m_lifetimeLocation = getUniformLocation("lifetime");
        m_fadeStartLocation = getUniformLocation("fadeStart");
        m_skidTextureLocation = getUniformLocation("skidTexture");
    }

    void SkidMarkShader::customCleanup() {
    }

    void SkidMarkShader::loadProjectionViewMatrices(glm::mat4 const &projection, glm::mat4 const &view) const {
        loadMat4(m_projectionMatrixLocation, &projection[0][0]);
        loadMat4(m_viewMatrixLocation, &view[0][0]);
    }

    void SkidMarkShader::loadLifetime(float const lifetime) const {
        loadFloat(m_lifetimeLocation, lifetime);
    }

    void SkidMarkShader::loadFadeStart(float const fadeStart) const {
        loadFloat(m_fadeStartLocation, fadeStart);
    }

    void SkidMarkShader::bindSkidTexture(GLuint const textureID) const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        loadSampler2D(m_skidTextureLocation, 0);
    }
} // namespace OpenNFS
