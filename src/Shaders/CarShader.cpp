//
// Created by Amrik on 01/03/2018.
//

#include "CarShader.h"
#include "../Scene/Light.h"

const std::string vertexSrc = "../shaders/CarVertexShader.vertexshader";
const std::string fragSrc = "../shaders/CarFragmentShader.fragmentshader";

CarShader::CarShader() : super(vertexSrc, fragSrc){
    bindAttributes();
    getAllUniformLocations();
    load_tga_texture("car00.tga");
}

void CarShader::bindAttributes() {
    bindAttribute(0 ,"vertexPosition_modelspace");
    bindAttribute(1 ,"vertexUV");
    bindAttribute(2 ,"normal");
}

void CarShader::getAllUniformLocations() {
    // Get handles for uniforms
    matrixLocation = getUniformLocation("MVP");
    textureLocation = getUniformLocation("myTextureSampler");
    colourLocation = getUniformLocation("carColour");
    lightPositionLocation = getUniformLocation("lightPosition");
    lightColourLocation = getUniformLocation("lightColour");
}

void CarShader::customCleanup(){
    glDeleteTextures(1, &TextureID);
}

void CarShader::load_tga_texture(const char *path) {
    NS_TGALOADER::IMAGE texture_loader;

    if (!texture_loader.LoadTGA(path)) {
        printf("Texture loading failed!\n");
        exit(2);
    }

    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void CarShader::loadCarTexture(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureID);
}

void CarShader::loadLight(Light light){
    loadVec3(lightPositionLocation, light.position);
    loadVec3(lightColourLocation, light.colour);
}

void CarShader::loadCarColor(glm::vec3 color){
    loadVec3(colourLocation, color);
}

void CarShader::loadMVPMatrix(glm::mat4 matrix){
    loadMat4(matrixLocation, &matrix[0][0]);
}


