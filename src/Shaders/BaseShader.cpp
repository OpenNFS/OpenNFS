#include <glm/vec3.hpp>
#include "BaseShader.h"
#include "../Config.h"
#include "../Util/Utils.h"

BaseShader::BaseShader(const std::string &vertex_file_path, const std::string &fragment_file_path) {
    // Prepended to shaders
    m_shaderSet.SetVersion(ONFS_GL_VERSION);

    // File prepended to shaders (after #version)
    m_shaderSet.SetPreambleFile(SHADER_PREAMBLE_PATH);

    m_programID = m_shaderSet.AddProgramFromExts({vertex_file_path, fragment_file_path});
    m_shaderSet.UpdatePrograms();
}

BaseShader::BaseShader(const std::string &vertex_file_path, const std::string &geometry_file_path, const std::string &fragment_file_path) {
    // Prepended to shaders
    m_shaderSet.SetVersion(ONFS_GL_VERSION);

    // File prepended to shaders (after #version)
    m_shaderSet.SetPreambleFile(SHADER_PREAMBLE_PATH);

    m_programID = m_shaderSet.AddProgramFromExts({vertex_file_path, geometry_file_path, fragment_file_path});
    m_shaderSet.UpdatePrograms();
}

void BaseShader::loadSampler2D(GLint location, GLint textureUnit) {
    glUniform1i(location, textureUnit);
}

void BaseShader::loadBool(GLint location, bool value) {
    glUniform1i(location, value);
}

void BaseShader::loadFloat(GLint location, float value) {
    glUniform1f(location, value);
}

void BaseShader::loadVec4(GLint location, glm::vec4 value) {
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void BaseShader::loadVec3(GLint location, glm::vec3 value) {
    glUniform3f(location, value.x, value.y, value.z);
}

void BaseShader::loadVec2(GLint location, glm::vec2 value) {
    glUniform2f(location, value.x, value.y);
}

void BaseShader::loadMat4(GLint location, const GLfloat *value) {
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void BaseShader::loadMat3(GLint location, const GLfloat *value) {
    glUniformMatrix3fv(location, 1, GL_FALSE, value);
}

void BaseShader::cleanup() {
    customCleanup();
}

void BaseShader::use() {
    glUseProgram(*m_programID);
}

void BaseShader::unbind() {
    glUseProgram(0);
}

void BaseShader::HotReload() {
    m_shaderSet.UpdatePrograms();
}

BaseShader::~BaseShader() {
    glDeleteProgram(*m_programID);
}

GLint BaseShader::getUniformLocation(std::string uniformName) {
    return glGetUniformLocation(*m_programID, uniformName.c_str());
}

void BaseShader::bindAttribute(GLuint attribute, std::string variableName) {
    glBindAttribLocation(*m_programID, attribute, variableName.c_str());
}