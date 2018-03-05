//
// Created by Amrik on 01/03/2018.
//
#include "TrackShader.h"

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
}

void TrackShader::customCleanup(){

}

void TrackShader::bindTrackTextures(Track track_block, std::map<short, GLuint> gl_id_map) {
    // TODO: Somehow breaking the CORE profile here?
    GLenum texNum = GL_TEXTURE0;
    for (short texture_id : track_block.texture_ids) {
        glActiveTexture(texNum++);
        glBindTexture(GL_TEXTURE_2D, gl_id_map.find(texture_id)->second);
    }
    const GLint samplers[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                24, 25, 26, 27, 28, 29, 30, 31};
    glUniform1iv(TrackTexturesID, 32, samplers);
}

void TrackShader::loadMatrices(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &transformation){
    loadMat4(viewMatrixLocation, &view[0][0]);
    loadMat4(projectionMatrixLocation, &projection[0][0]);
    loadMat4(transformationMatrixLocation, &transformation[0][0]);
}


