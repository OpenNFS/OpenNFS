//
// Created by Amrik Sadhra on 01/03/2018.
//

#include <glm/vec3.hpp>
#include "BaseShader.h"
#include "../Util/Utils.h"

BaseShader::BaseShader(const std::string &vertex_file_path, const std::string &fragment_file_path) {
    // Create the shaders
    VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);

    ASSERT(VertexShaderStream.is_open(), "Impossible to open! " << vertex_file_path);
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
    LOG(INFO) << "Compiling shader : " << vertex_file_path.c_str();
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
        LOG(WARNING) << &VertexShaderErrorMessage[0];
    }

    // Compile Fragment Shader
    LOG(INFO) << "Compiling shader : " << fragment_file_path.c_str();
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
        LOG(WARNING) << &FragmentShaderErrorMessage[0];
    }

    // Link the program
    LOG(INFO) << "Linking program";

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
        LOG(WARNING) << &ProgramErrorMessage[0];
    }
}

BaseShader::BaseShader(const std::string &vertex_file_path, const std::string &geometry_file_path, const std::string &fragment_file_path) {
    // Create the shaders
    VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);

    ASSERT(VertexShaderStream.is_open(), "Impossible to open! " << vertex_file_path);
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

    // Read the Geometry Shader code from the file
    std::string GeometryShaderCode;
    std::ifstream GeometryShaderStream(geometry_file_path, std::ios::in);
    if(GeometryShaderStream.is_open()){
        std::string Line;
        while(getline(GeometryShaderStream, Line))
            GeometryShaderCode += "\n" + Line;
        GeometryShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    LOG(INFO) << "Compiling shader : " << vertex_file_path.c_str();
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
        LOG(WARNING) << &VertexShaderErrorMessage[0];
    }

    // Compile Fragment Shader
    LOG(INFO) << "Compiling shader : " << fragment_file_path.c_str();
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
        LOG(WARNING) << &FragmentShaderErrorMessage[0];
    }

    // Compile Geometry Shader
    LOG(INFO) << "Compiling shader : " << geometry_file_path.c_str();
    char const * GeometrySourcePointer = GeometryShaderCode.c_str();
    glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , nullptr);
    glCompileShader(GeometryShaderID);

    // Check Fragment Shader
    glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> GeometryShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(GeometryShaderID, InfoLogLength, nullptr, &GeometryShaderErrorMessage[0]);
        LOG(WARNING) << &GeometryShaderErrorMessage[0];
    }

    // Link the program
    LOG(INFO) << "Linking program";

    ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glAttachShader(ProgramID, GeometryShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
        LOG(WARNING) << &ProgramErrorMessage[0];
    }
}

void BaseShader::loadSampler2D(GLint location, GLint textureUnit){
    glUniform1i(location, textureUnit);
}

void BaseShader::loadBool(GLint location, bool value){
    glUniform1i(location, value);
}

void BaseShader::loadFloat(GLint location, float value){
    glUniform1f(location, value);
}

void BaseShader::loadVec4(GLint location, glm::vec4 value){
    glUniform4f(location, value.x, value.y, value.z, value.w);
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
    return glGetUniformLocation(ProgramID, uniformName.c_str());
}

void BaseShader::bindAttribute(GLuint attribute, std::string variableName) {
    glBindAttribLocation(ProgramID, attribute, variableName.c_str());
}

