//
// Created by Amrik Sadhra on 16/10/2018.
//

#include "DepthShader.h"

const std::string vertexSrc = "../shaders/DepthVertexShader.vertexshader";
const std::string fragSrc = "../shaders/DepthFragmentShader.fragmentshader";

DepthShader::DepthShader() : super(vertexSrc, fragSrc){
    bindAttributes();
    getAllUniformLocations();
}

void DepthShader::bindAttributes() {
    bindAttribute(0 ,"vertexPosition_modelspace");
    bindAttribute(1 ,"vertexUV");
    bindAttribute(2 ,"normal");
}

void DepthShader::getAllUniformLocations() {
    lightSpaceMatrixLocation = getUniformLocation("lightSpaceMatrix");
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
}


void DepthShader::customCleanup() {

}

void DepthShader::loadLightSpaceMatrix(const glm::mat4 &lightSpaceMatrix) {
    loadMat4(lightSpaceMatrixLocation, &lightSpaceMatrix[0][0]);
}

void DepthShader::loadTransformMatrix(const glm::mat4 &transformationMatrix) {
    loadMat4(transformationMatrixLocation, &transformationMatrix[0][0]);
}