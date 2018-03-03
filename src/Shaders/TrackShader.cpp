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
    bindAttribute(2 ,"normal");
    bindAttribute(3 ,"nfsData");
}

void TrackShader::getAllUniformLocations() {
    // Get handles for uniforms
    MatrixID = getUniformLocation("MVP");
    TrackTexturesID = getUniformLocation("texture_array");
}

void TrackShader::customCleanup(){

}

void TrackShader::bindTrackTextures(Model track_block, std::map<short, GLuint> gl_id_map) {
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

void TrackShader::loadMVPMatrix(glm::mat4 matrix){
    loadMat4(MatrixID, &matrix[0][0]);
}


