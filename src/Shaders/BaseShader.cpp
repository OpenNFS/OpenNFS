#include <glm/vec3.hpp>
#include "BaseShader.h"
#include "../Config.h"
#include "../Util/Utils.h"

BaseShader::BaseShader(const std::string &vertex_file_path, const std::string &fragment_file_path)
{
    // Prepended to shaders
    shaderSet.SetVersion(ONFS_GL_VERSION);

    // File prepended to shaders (after #version)
    shaderSet.SetPreambleFile(SHADER_PREAMBLE_PATH);

    ProgramID = shaderSet.AddProgramFromExts({vertex_file_path, fragment_file_path});
    shaderSet.UpdatePrograms();
}

BaseShader::BaseShader(const std::string &vertex_file_path, const std::string &geometry_file_path, const std::string &fragment_file_path)
{
    // Prepended to shaders
    shaderSet.SetVersion(ONFS_GL_VERSION);

    // File prepended to shaders (after #version)
    shaderSet.SetPreambleFile(SHADER_PREAMBLE_PATH);

    ProgramID = shaderSet.AddProgramFromExts({vertex_file_path, geometry_file_path, fragment_file_path});
    shaderSet.UpdatePrograms();
}

void BaseShader::loadSampler2D(GLint location, GLint textureUnit)
{
    glUniform1i(location, textureUnit);
}

void BaseShader::loadBool(GLint location, bool value)
{
    glUniform1i(location, value);
}

void BaseShader::loadFloat(GLint location, float value)
{
    glUniform1f(location, value);
}

void BaseShader::loadVec4(GLint location, glm::vec4 value)
{
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void BaseShader::loadVec3(GLint location, glm::vec3 value)
{
    glUniform3f(location, value.x, value.y, value.z);
}

void BaseShader::loadVec2(GLint location, glm::vec2 value)
{
    glUniform2f(location, value.x, value.y);
}

void BaseShader::loadMat4(GLint location, const GLfloat *value)
{
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void BaseShader::loadMat3(GLint location, const GLfloat *value)
{
    glUniformMatrix3fv(location, 1, GL_FALSE, value);
}

void BaseShader::cleanup()
{
    customCleanup();
}

void BaseShader::use()
{
    glUseProgram(*ProgramID);
}

void BaseShader::unbind()
{
    glUseProgram(0);
}

BaseShader::~BaseShader()
{
    glDeleteProgram(*ProgramID);
}

GLint BaseShader::getUniformLocation(std::string uniformName)
{
    return glGetUniformLocation(*ProgramID, uniformName.c_str());
}

void BaseShader::bindAttribute(GLuint attribute, std::string variableName)
{
    glBindAttribLocation(*ProgramID, attribute, variableName.c_str());
}
