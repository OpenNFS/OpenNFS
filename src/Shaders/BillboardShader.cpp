//
// Created by Amrik Sadhra on 11/03/2018.
//

#include "BillboardShader.h"

const std::string vertexSrc = "../shaders/BillboardVertexShader.vertexshader";
const std::string fragSrc = "../shaders/BillboardFragmentShader.fragmentshader";

BillboardShader::BillboardShader() : super(vertexSrc, fragSrc){
    bindAttributes();
    getAllUniformLocations();
    load_bmp_texture();
}

void BillboardShader::bindAttributes() {
    bindAttribute(0 ,"vertexPosition_modelspace");
    bindAttribute(1 ,"vertexUV");
    bindAttribute(2 ,"normal");
}

void BillboardShader::getAllUniformLocations() {
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    projectionMatrixLocation = getUniformLocation("projectionMatrix");
    viewMatrixLocation = getUniformLocation("viewMatrix");
    boardTextureLocation = getUniformLocation("boardTextureSampler");
    lightColourLocation = getUniformLocation("lightColour");
    billboardPosLocation = getUniformLocation("billboardPos");
}

void BillboardShader::loadBillboardTexture(){
    loadSampler2D(boardTextureLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void BillboardShader::loadLight(Light board_light) {
    loadVec4(lightColourLocation, board_light.colour);
    loadVec3(billboardPosLocation, board_light.position);
    loadBillboardTexture();
}

void BillboardShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation) {
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}


void BillboardShader::customCleanup() {
    glDeleteTextures(1, &textureID);
}

void BillboardShader::load_bmp_texture() {
    GLubyte *data;
    GLsizei width = 64;
    GLsizei height = 64;

    Utils::LoadBmpWithAlpha("../resources/sfx/0004.bmp", "../resources/sfx/0004-a.bmp", &data, width, height);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *) data);
}
