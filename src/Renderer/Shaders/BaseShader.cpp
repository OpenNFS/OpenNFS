#include "BaseShader.h"
#include "../../Config.h"
#include "../../Util/Utils.h"
#include <glm/vec3.hpp>

namespace OpenNFS {
    BaseShader::BaseShader(std::string const &vertex_file_path, std::string const &fragment_file_path) {
        // Prepended to shaders
        m_shaderSet.SetVersion(ONFS_GL_VERSION);

        // File prepended to shaders (after #version)
        m_shaderSet.SetPreambleFile(SHADER_PREAMBLE_PATH);

        m_programID = m_shaderSet.AddProgramFromExts({vertex_file_path, fragment_file_path});
        m_shaderSet.UpdatePrograms();
    }

    BaseShader::BaseShader(std::string const &vertex_file_path,
                           std::string const &geometry_file_path,
                           std::string const &fragment_file_path) {
        // Prepended to shaders
        m_shaderSet.SetVersion(ONFS_GL_VERSION);

        // File prepended to shaders (after #version)
        m_shaderSet.SetPreambleFile(SHADER_PREAMBLE_PATH);

        m_programID = m_shaderSet.AddProgramFromExts({vertex_file_path, geometry_file_path, fragment_file_path});
        m_shaderSet.UpdatePrograms();
    }

    BaseShader::~BaseShader() {
        glDeleteProgram(*m_programID);
    }

    void BaseShader::use() const {
        glUseProgram(*m_programID);
    }

    void BaseShader::unbind() {
        glUseProgram(0);
    }

    void BaseShader::cleanup() {
        customCleanup();
    }

    void BaseShader::HotReload() {
        m_shaderSet.UpdatePrograms();
    }

    void BaseShader::loadMat4(GLint const location, GLfloat const *value) {
        glUniformMatrix4fv(location, 1, GL_FALSE, value);
    }

    void BaseShader::loadMat3(GLint const location, GLfloat const *value) {
        glUniformMatrix3fv(location, 1, GL_FALSE, value);
    }

    void BaseShader::loadUint(GLint const location, GLuint const value) {
        glUniform1ui(location, value);
    }

    void BaseShader::loadVec4(GLint const location, glm::vec4 const value) {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }

    void BaseShader::loadVec2(GLint const location, glm::vec2 const value) {
        glUniform2f(location, value.x, value.y);
    }

    void BaseShader::loadVec3(GLint const location, glm::vec3 const value) {
        glUniform3f(location, value.x, value.y, value.z);
    }

    void BaseShader::loadFloat(GLint const location, GLfloat const value) {
        glUniform1f(location, value);
    }

    void BaseShader::loadSampler2D(GLint const location, GLint const textureUnit) {
        glUniform1i(location, textureUnit);
    }

    GLint BaseShader::getUniformLocation(std::string const &uniformName) const {
        return glGetUniformLocation(*m_programID, uniformName.c_str());
    }

    void BaseShader::bindAttribute(GLuint const attribute, std::string const &variableName) const {
        glBindAttribLocation(*m_programID, attribute, variableName.c_str());
    }
} // namespace OpenNFS
