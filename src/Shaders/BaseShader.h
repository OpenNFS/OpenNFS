#pragma once

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <g3log/g3log.hpp>
#include <glm/vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <GL/glew.h>
#include "ShaderSet.h"

using namespace std;

class BaseShader {
public:
    BaseShader(const std::string &vertex_file_path, const std::string &fragment_file_path);
    BaseShader(const std::string &vertex_file_path, const std::string &geometry_file_path, const std::string &fragment_file_path);
    ~BaseShader();
    void use();
    void unbind();
    void cleanup();
    void HotReload();

protected:
    void loadMat4(GLint location, const GLfloat *value);
    void loadMat3(GLint location, const GLfloat *value);
    void loadBool(GLint location, bool value);
    void loadVec4(GLint location, glm::vec4 value);
    void loadVec2(GLint location, glm::vec2 value);
    void loadVec3(GLint location, glm::vec3 value);
    void loadFloat(GLint location, float value);
    void loadSampler2D(GLint location, GLint textureUnit);
    GLint getUniformLocation(string uniformName);
    void bindAttribute(GLuint attribute, std::string variableName);
    virtual void bindAttributes()         = 0;
    virtual void getAllUniformLocations() = 0;
    virtual void customCleanup()          = 0;

private:
    GLuint *m_programID;
    ShaderSet m_shaderSet;
};
