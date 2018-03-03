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
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    projectionMatrixLocation = getUniformLocation("projectionMatrix");
    viewMatrixLocation = getUniformLocation("viewMatrix");
    textureLocation = getUniformLocation("myTextureSampler");
    colourLocation = getUniformLocation("colour");
    lightPositionLocation = getUniformLocation("lightPosition");
    lightColourLocation = getUniformLocation("lightColour");
    shineDamperLocation=  getUniformLocation("shineDamper");
    reflectivityLocation =  getUniformLocation("reflectivity");
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
    loadSampler2D(textureLocation, TextureID);
}

void CarShader::loadSpecular(float damper, float reflectivity){
    loadFloat(shineDamperLocation, damper);
    loadFloat(reflectivityLocation, reflectivity);
}

void CarShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation){
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}

void CarShader::loadLight(Light light){
    loadVec3(lightPositionLocation, light.position);
    loadVec3(lightColourLocation, light.colour);
}

void CarShader::loadCarColor(glm::vec3 color){
    loadVec3(colourLocation, color);
}


