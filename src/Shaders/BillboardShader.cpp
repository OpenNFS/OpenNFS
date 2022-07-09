#include "BillboardShader.h"

const std::string vertexSrc = "../shaders/BillboardVertexShader.vert";
const std::string fragSrc   = "../shaders/BillboardFragmentShader.frag";

BillboardShader::BillboardShader() : super(vertexSrc, fragSrc) {
    bindAttributes();
    getAllUniformLocations();
    load_bmp_texture();
}

void BillboardShader::bindAttributes() {
    bindAttribute(0, "vertexPosition_modelspace");
    bindAttribute(1, "vertexUV");
}

void BillboardShader::getAllUniformLocations() {
    projectionMatrixLocation = getUniformLocation("projectionMatrix");
    viewMatrixLocation       = getUniformLocation("viewMatrix");
    boardTextureLocation     = getUniformLocation("boardTextureSampler");
    lightColourLocation      = getUniformLocation("lightColour");
    billboardPosLocation     = getUniformLocation("billboardPos");
}

void BillboardShader::loadBillboardTexture() {
    loadSampler2D(boardTextureLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void BillboardShader::loadLight(const std::shared_ptr<TrackLight> &light) {
    loadVec4(lightColourLocation, light->colour);
    loadVec3(billboardPosLocation, light->position);
    loadBillboardTexture();
}

void BillboardShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view) {
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
}

void BillboardShader::customCleanup() {
    glDeleteTextures(1, &textureID);
}

void BillboardShader::load_bmp_texture() {
    GLubyte *data;
    GLsizei width;
    GLsizei height;

    ImageLoader::LoadBmpWithAlpha("../resources/sfx/0004.BMP", "../resources/sfx/0004-a.BMP", &data, &width, &height);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *) data);
}
