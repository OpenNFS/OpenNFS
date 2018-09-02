//
// Created by Amrik Sadhra on 01/03/2018.
//

#include "CarShader.h"

const std::string vertexSrc = "../shaders/CarVertexShader.vertexshader";
const std::string fragSrc = "../shaders/CarFragmentShader.fragmentshader";

CarShader::CarShader(shared_ptr<Car> current_car) : super(vertexSrc, fragSrc){
    car = current_car;
    bindAttributes();
    getAllUniformLocations();
    //load_tga_texture();
    LoadEnvMapTexture();
}

void CarShader::LoadEnvMapTexture() {
    std::stringstream filename;
    filename << "../resources/misc/sky_textures/CHRD.BMP";

    int width, height;
    GLubyte *data;

    ASSERT(Utils::LoadBmpCustomAlpha(filename.str().c_str(), &data, &width, &height, 0), "Environment map texture loading failed!");

    glGenTextures(1, &envMapTextureID);
    glBindTexture(GL_TEXTURE_2D, envMapTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *) data);
}

void CarShader::bindAttributes() {
    bindAttribute(0 ,"vertexPosition_modelspace");
    bindAttribute(1 ,"vertexUV");
    bindAttribute(2 ,"normal");
    bindAttribute(3 ,"textureIndex");
    bindAttribute(4 ,"polygonFlag");
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
    envReflectivityLocation  =  getUniformLocation("envReflectivity");
    CarTexturesID = getUniformLocation("texture_array");
    isMultiTexturedLocation = getUniformLocation("multiTextured");
    hasPolyFlagsLocation = getUniformLocation("polyFlagged");
}

void CarShader::setMultiTextured(bool multiTextured){
    loadBool(isMultiTexturedLocation, multiTextured);
}

void CarShader::setPolyFlagged(bool polyFlagged){
    loadBool(hasPolyFlagsLocation, polyFlagged);
}

void CarShader::customCleanup(){
    glDeleteTextures(1, &textureID);
}

void CarShader::bindCarTextures(const CarModel &car_model, std::map<unsigned int, GLuint> gl_id_map) {
    GLenum texNum = GL_TEXTURE0;
    for (unsigned int texture_id : car_model.texture_ids) {
        glActiveTexture(texNum++);
        glBindTexture(GL_TEXTURE_2D, gl_id_map.find(texture_id)->second);
        if(texNum - GL_TEXTURE0 > 14){
            break;
            //std::cerr << "Too many textures in Car for number of GPU samplers" << std::endl;
        }
    }
    const GLint samplers[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    glUniform1iv(CarTexturesID, 14, samplers);
}

void CarShader::load_tga_texture() {
    std::stringstream car_texture_path;
    car_texture_path << CAR_PATH << ToString(car->tag) << "/" <<car->name << "/car00.tga";

    NS_TGALOADER::IMAGE texture_loader;
    ASSERT(texture_loader.LoadTGA(car_texture_path.str().c_str()), "Car Texture loading failed!");

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_loader.getWidth(), texture_loader.getHeight(), 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_loader.getDataForOpenGL());

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

void CarShader::loadSpecular(float damper, float reflectivity, float env_reflectivity){
    loadFloat(shineDamperLocation, damper);
    loadFloat(reflectivityLocation, reflectivity);
    loadFloat(envReflectivityLocation, env_reflectivity);
}

void CarShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation){
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}

void CarShader::loadLight(Light light){
    loadVec3(lightPositionLocation, light.position);
    loadVec4(lightColourLocation, light.colour);
}

void CarShader::loadCarColor(glm::vec3 color){
    loadVec3(colourLocation, color);
}



