#pragma once

#include <glm/detail/type_mat4x4.hpp>
#include <glm/vec3.hpp>

#include "ShaderSet.h"

namespace OpenNFS {
    class BaseShader {
      public:
        BaseShader(std::string const &vertex_file_path, std::string const &fragment_file_path);
        BaseShader(std::string const &vertex_file_path, std::string const &geometry_file_path, std::string const &fragment_file_path);

        virtual ~BaseShader();

        void use() const;
        static void unbind();
        void cleanup();
        void HotReload();

      protected:
        static void loadMat4(GLint location, GLfloat const *value);
        static void loadMat3(GLint location, GLfloat const *value);
        static void loadUint(GLint location, GLuint value);
        static void loadVec4(GLint location, glm::vec4 value);
        static void loadVec2(GLint location, glm::vec2 value);
        static void loadVec3(GLint location, glm::vec3 value);
        static void loadFloat(GLint location, GLfloat value);
        static void loadSampler2D(GLint location, GLint textureUnit);

        [[nodiscard]] GLint getUniformLocation(std::string const &uniformName) const;
        void bindAttribute(GLuint attribute, std::string const &variableName) const;

        virtual void bindAttributes() = 0;
        virtual void getAllUniformLocations() = 0;
        virtual void customCleanup() = 0;

      private:
        GLuint *m_programID;
        ShaderSet m_shaderSet;
    };
} // namespace OpenNFS
