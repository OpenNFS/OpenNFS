//
// Created by Amrik on 01/03/2018.
//

#include <glm/vec3.hpp>
#include "BaseShader.h"
#include "../Util/Assert.h"

BaseShader::BaseShader(const std::string &vertex_file_path, const std::string &fragment_file_path) {
    // Create the shaders
    VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);

    ASSERT(VertexShaderStream.is_open(), "Impossible to open %s!\n", vertex_file_path.c_str());
    std::string Line;
    while(getline(VertexShaderStream, Line))
        VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line;
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path.c_str());
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path.c_str());
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");

    ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
}

void BaseShader::loadSampler2D(GLint location, GLint textureUnit){
    glUniform1i(location, textureUnit);
}

void BaseShader::loadFloat(GLint location, float value){
    glUniform1f(location, value);
}

void BaseShader::loadVec3(GLint location, glm::vec3 value){
    glUniform3f(location, value.x, value.y, value.z);
}


void BaseShader::loadVec2(GLint location, glm::vec2 value){
    glUniform2f(location, value.x, value.y);
}

void BaseShader::loadMat4(GLint location, const GLfloat *value){
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void BaseShader::cleanup(){
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    customCleanup();
}

void BaseShader::use(){
    glUseProgram(ProgramID);
}

void BaseShader::unbind(){
    glUseProgram(0);
}

BaseShader::~BaseShader(){
    glDeleteProgram(ProgramID);
}

GLint BaseShader::getUniformLocation(std::string uniformName){
    glGetUniformLocation(ProgramID, uniformName.c_str());
}

void BaseShader::bindAttribute(GLuint attribute, std::string variableName) {
    glBindAttribLocation(ProgramID, attribute, variableName.c_str());
}
