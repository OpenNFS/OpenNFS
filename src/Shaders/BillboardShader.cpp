//
// Created by Amrik on 11/03/2018.
//

#include "BillboardShader.h"

const std::string vertexSrc = "../shaders/BillboardVertexShader.vertexshader";
const std::string fragSrc = "../shaders/BillboardFragmentShader.fragmentshader";

BillboardShader::BillboardShader() : super(vertexSrc, fragSrc){
    bindAttributes();
    getAllUniformLocations();
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

void BillboardShader::loadLight(Light board_light) {
    loadVec3(lightColourLocation, board_light.colour);
    loadVec3(billboardPosLocation, glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) * board_light.position);
}

void BillboardShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation) {
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}


void BillboardShader::customCleanup() {
    glDeleteTextures(1, &textureID);
}
