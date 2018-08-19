//
// Created by Amrik on 01/03/2018.
//
#include <memory>
#include "TrackShader.h"
#include "../Util/Utils.h"

const std::string vertexSrc = "../shaders/TrackVertexShader.vertexshader";
const std::string fragSrc = "../shaders/TrackFragmentShader.fragmentshader";

TrackShader::TrackShader() : super(vertexSrc, fragSrc){
    bindAttributes();
    getAllUniformLocations();
}

void TrackShader::bindAttributes() {
    bindAttribute(0 ,"vertexPosition_modelspace");
    bindAttribute(1 ,"vertexUV");
    bindAttribute(2 ,"textureIndex");
    bindAttribute(3 ,"nfsData");
    bindAttribute(4 ,"normal");
}

void TrackShader::getAllUniformLocations() {
    // Get handles for uniforms
    transformationMatrixLocation = getUniformLocation("transformationMatrix");
    projectionMatrixLocation = getUniformLocation("projectionMatrix");
    viewMatrixLocation = getUniformLocation("viewMatrix");
    TrackTexturesID = getUniformLocation("texture_array");
    shineDamperLocation=  getUniformLocation("shineDamper");
    reflectivityLocation =  getUniformLocation("reflectivity");
    useClassicLocation = getUniformLocation("useClassic");

    for(int i = 0; i < MAX_LIGHTS; ++i){
        lightPositionLocation[i] = getUniformLocation("lightPosition[" + std::to_string(i) + "]");
        lightColourLocation[i] =  getUniformLocation("lightColour[" + std::to_string(i) + "]");
        attenuationLocation[i] = getUniformLocation("attenuation[" + std::to_string(i) + "]");
    }
}

void TrackShader::customCleanup(){

}

void TrackShader::bindTrackTextures(const Track &track_block, std::map<short, GLuint> gl_id_map) {
    // TODO: Somehow breaking the CORE profile here?
    GLenum texNum = GL_TEXTURE0;
    for (short texture_id : track_block.texture_ids) {
        glActiveTexture(texNum++);
        glBindTexture(GL_TEXTURE_2D, gl_id_map.find(texture_id)->second);
       if(texNum - GL_TEXTURE0 > 32){
           //std::cerr << "Too many textures in Trackblock for number of GPU samplers" << std::endl;
       }
    }
    const GLint samplers[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    glUniform1iv(TrackTexturesID, 32, samplers);
}

void TrackShader::loadLights(std::vector<Light> lights) {
    for(int i = 0; i < MAX_LIGHTS; ++i){
        if(i < lights.size()){
            loadVec3(lightPositionLocation[i], lights[i].position);
            loadVec4(lightColourLocation[i], lights[i].colour);
            loadVec3(attenuationLocation[i], lights[i].attenuation);
        } else {
            loadVec3(lightPositionLocation[i], glm::vec3(0,0,0));
            loadVec4(lightColourLocation[i], glm::vec4(0,0,0,0));
            loadVec3(attenuationLocation[i], glm::vec3(1,0,0));
        }
    }
}

void TrackShader::setClassic(bool useClassic){
    loadBool(useClassicLocation, useClassic);
}

void TrackShader::loadSpecular(float damper, float reflectivity){
    loadFloat(shineDamperLocation, damper);
    loadFloat(reflectivityLocation, reflectivity);
}

void TrackShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation){
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}


