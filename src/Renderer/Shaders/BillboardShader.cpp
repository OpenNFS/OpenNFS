#include "BillboardShader.h"

namespace OpenNFS {
    std::string const vertexSrc = "../shaders/BillboardShader.vert";
    std::string const fragSrc = "../shaders/BillboardShader.frag";

    BillboardShader::BillboardShader() : BaseShader(vertexSrc, fragSrc) {
        BillboardShader::bindAttributes();
        BillboardShader::getAllUniformLocations();
        load_bmp_texture();
    }

    void BillboardShader::bindAttributes() {
        bindAttribute(0, "vertexPosition_modelspace");
        bindAttribute(1, "vertexUV");
    }

    void BillboardShader::getAllUniformLocations() {
        projectionMatrixLocation = getUniformLocation("projectionMatrix");
        viewMatrixLocation = getUniformLocation("viewMatrix");
        boardTextureLocation = getUniformLocation("boardTextureSampler");
        lightColourLocation = getUniformLocation("lightColour");
        billboardPosLocation = getUniformLocation("billboardPos");
    }

    void BillboardShader::loadBillboardTexture() const {
        loadSampler2D(boardTextureLocation, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    void BillboardShader::loadLight(LibOpenNFS::TrackLight const *light) const {
        loadVec4(lightColourLocation, light->colour);
        loadVec3(billboardPosLocation, light->geometry.position);
        loadBillboardTexture();
    }

    void BillboardShader::loadMatrices(glm::mat4 const &projection, glm::mat4 const &view) const {
        loadMat4(viewMatrixLocation, &view[0][0]);
        loadMat4(projectionMatrixLocation, &projection[0][0]);
    }

    void BillboardShader::customCleanup() {
        glDeleteTextures(1, &textureID);
    }

    void BillboardShader::load_bmp_texture() {
        std::vector<uint8_t> data;
        GLsizei width;
        GLsizei height;

        ImageLoader::LoadBmpWithAlpha("../resources/sfx/0004.BMP", "../resources/sfx/0004-a.BMP", data, &width, &height);
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    }
} // namespace OpenNFS
