//
// Created by Amrik on 01/03/2018.
//

#include "CarShader.h"
#include "../Loaders/trk_loader.h"

const std::string vertexSrc = "../shaders/CarVertexShader.vertexshader";
const std::string fragSrc = "../shaders/CarFragmentShader.fragmentshader";

CarShader::CarShader() : super(vertexSrc, fragSrc){
    bindAttributes();
    getAllUniformLocations();
    load_tga_texture("car00.tga");
    LoadEnvMapTexture();
}

void CarShader::LoadEnvMapTexture() {
    int width, height;
    unsigned char *data;
    FILE *file;
    std::stringstream filename;
    filename << "../resources/TRK006/sky_textures/" << "0004.BMP";
    file = fopen(filename.str().c_str(), "rb");
    if (file == nullptr) {
        std::cout << "Couldn't open " << filename.str() << std::endl;
        assert(file == nullptr);
    }

    width = 128;
    height = 128;

    data = (unsigned char *) malloc(width * height * 3);
    fseek(file, 54, SEEK_SET);
    fread(data, width * height * 3, 1, file);
    fclose(file);

    for (int i = 0; i < width * height; ++i) {
        int index = i * 3;
        unsigned char B, R;
        B = data[index];
        R = data[index + 2];
        data[index] = R;
        data[index + 2] = B;
    }

    glGenTextures(1, &envMapTextureID);
    glBindTexture(GL_TEXTURE_2D, envMapTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 128, 128, GL_RGB, GL_UNSIGNED_BYTE,
                      data);
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
    envMapTextureLocation = getUniformLocation("envMapTextureSampler");
    carTextureLocation = getUniformLocation("carTextureSampler");
    colourLocation = getUniformLocation("colour");
    lightPositionLocation = getUniformLocation("lightPosition");
    lightColourLocation = getUniformLocation("lightColour");
    shineDamperLocation=  getUniformLocation("shineDamper");
    reflectivityLocation =  getUniformLocation("reflectivity");
}

void CarShader::customCleanup(){
    glDeleteTextures(1, &textureID);
}

void CarShader::load_tga_texture(const char *path) {
    NS_TGALOADER::IMAGE texture_loader;

    if (!texture_loader.LoadTGA(path)) {
        printf("Texture loading failed!\n");
        exit(2);
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void CarShader::loadCarTexture(){
    loadSampler2D(carTextureLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    loadEnvironmentMapTexture();
}

void CarShader::loadEnvironmentMapTexture(){
    loadSampler2D(envMapTextureLocation, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, envMapTextureID);
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


