//
// Created by Amrik Sadhra on 21/08/2018.
//

#include "SkydomeShader.h"

const std::string vertexSrc = "../shaders/SkydomeVertexShader.vertexshader";
const std::string fragSrc = "../shaders/SkydomeFragmentShader.fragmentshader";

SkydomeShader::SkydomeShader() : super(vertexSrc, fragSrc){
    bindAttributes();
    getAllUniformLocations();
}

void SkydomeShader::bindAttributes() {
    bindAttribute(0 ,"vertexPosition_modelspace");
    bindAttribute(1 ,"vertexUV");
    bindAttribute(2 ,"normal");
}

void SkydomeShader::getAllUniformLocations() {
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    projectionMatrixLocation = getUniformLocation("projectionMatrix");
    viewMatrixLocation = getUniformLocation("viewMatrix");


    lightColourLocation = getUniformLocation("lightColour");
    billboardPosLocation = getUniformLocation("billboardPos");
}

void SkydomeShader::loadLight(Light board_light) {
    loadVec4(lightColourLocation, board_light.colour);
    loadVec3(billboardPosLocation, board_light.position);
}

void SkydomeShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation) {
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}


void SkydomeShader::customCleanup() {

}
