#include "GLCarModel.h"
#include "../../Util/Utils.h"

namespace OpenNFS {
    GLCarModel::GLCarModel(CarGeometry const &geometry, float const specular_damper, float const specular_reflectivity,
                           float const env_reflectivity)
        : GLModel(), CarGeometry(geometry) {
        // Can't call basic constructor as genBuffers() call would run before m_textureIndices was available
        specularDamper = specular_damper;
        specularReflectivity = specular_reflectivity;
        envReflectivity = env_reflectivity;
    }

    GLCarModel::GLCarModel(CarGeometry const &geometry) : GLModel(), CarGeometry(geometry) {
    }

    GLCarModel::GLCarModel(GLCarModel &&other) noexcept
        : GLModel(), CarGeometry(std::move(other)), specularDamper(other.specularDamper), specularReflectivity(other.specularReflectivity),
          envReflectivity(other.envReflectivity), vertexBuffer(other.vertexBuffer), uvBuffer(other.uvBuffer),
          normalBuffer(other.normalBuffer), textureIndexBuffer(other.textureIndexBuffer), polyFlagBuffer(other.polyFlagBuffer) {
        // Transfer ownership of GL resources
        VertexArrayID = other.VertexArrayID;
        buffersGenerated = other.buffersGenerated;
        enabled = other.enabled;
        ModelMatrix = other.ModelMatrix;
        RotationMatrix = other.RotationMatrix;
        TranslationMatrix = other.TranslationMatrix;
        // Invalidate source
        other.VertexArrayID = 0;
        other.vertexBuffer = 0;
        other.uvBuffer = 0;
        other.normalBuffer = 0;
        other.textureIndexBuffer = 0;
        other.polyFlagBuffer = 0;
        other.buffersGenerated = false;
    }

    GLCarModel &GLCarModel::operator=(GLCarModel &&other) noexcept {
        if (this != &other) {
            // Clean up existing resources
            if (buffersGenerated) {
                glDeleteBuffers(1, &vertexBuffer);
                glDeleteBuffers(1, &uvBuffer);
                glDeleteBuffers(1, &normalBuffer);
                glDeleteBuffers(1, &textureIndexBuffer);
                glDeleteBuffers(1, &polyFlagBuffer);
                glDeleteVertexArrays(1, &VertexArrayID);
            }
            // Move base class
            CarGeometry::operator=(std::move(other));
            // Transfer ownership
            specularDamper = other.specularDamper;
            specularReflectivity = other.specularReflectivity;
            envReflectivity = other.envReflectivity;
            vertexBuffer = other.vertexBuffer;
            uvBuffer = other.uvBuffer;
            normalBuffer = other.normalBuffer;
            textureIndexBuffer = other.textureIndexBuffer;
            polyFlagBuffer = other.polyFlagBuffer;
            VertexArrayID = other.VertexArrayID;
            buffersGenerated = other.buffersGenerated;
            enabled = other.enabled;
            ModelMatrix = other.ModelMatrix;
            RotationMatrix = other.RotationMatrix;
            TranslationMatrix = other.TranslationMatrix;
            // Invalidate source
            other.VertexArrayID = 0;
            other.vertexBuffer = 0;
            other.uvBuffer = 0;
            other.normalBuffer = 0;
            other.textureIndexBuffer = 0;
            other.polyFlagBuffer = 0;
            other.buffersGenerated = false;
        }
        return *this;
    }

    GLCarModel::~GLCarModel() {
        if (buffersGenerated) {
            glDeleteBuffers(1, &vertexBuffer);
            glDeleteBuffers(1, &uvBuffer);
            glDeleteBuffers(1, &normalBuffer);
            glDeleteBuffers(1, &textureIndexBuffer);
            glDeleteBuffers(1, &polyFlagBuffer);
        }
    }

    void GLCarModel::UpdateMatrices() {
        if (!buffersGenerated) {
            buffersGenerated = true;
            CHECK_F(GenBuffers(), "Unable to generate GL Buffers for %s Model", name.c_str());
        }
        RotationMatrix = glm::toMat4(orientation);
        TranslationMatrix = glm::translate(glm::mat4(1.0), position);
        ModelMatrix = TranslationMatrix * RotationMatrix;
    }

    void GLCarModel::Render() {
        if (enabled) {
            glBindVertexArray(VertexArrayID);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices.size());
            glBindVertexArray(0);
        }
    }

    bool GLCarModel::GenBuffers() {
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        // Verts
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &(m_vertices[0]), GL_STATIC_DRAW);
        glVertexAttribPointer(0,              // attribute
                              3,              // size
                              GL_FLOAT,       // type
                              GL_FALSE,       // normalized?
                              0,              // stride
                              (void *)nullptr // array buffer offset
        );
        // 1st attribute buffer : Vertices
        glEnableVertexAttribArray(0);
        // UVs
        glGenBuffers(1, &uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &(m_uvs[0]), GL_STATIC_DRAW);
        glVertexAttribPointer(1,              // attribute
                              2,              // size
                              GL_FLOAT,       // type
                              GL_FALSE,       // normalized?
                              0,              // stride
                              (void *)nullptr // array buffer offset
        );
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        // Normals
        glGenBuffers(1, &normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &(m_normals[0]), GL_STATIC_DRAW);
        glVertexAttribPointer(2,              // attribute
                              3,              // size
                              GL_FLOAT,       // type
                              GL_TRUE,        // normalized?
                              0,              // stride
                              (void *)nullptr // array buffer offset
        );
        // 3rd attribute buffer : Normals
        glEnableVertexAttribArray(2);
        // Texture Indices
        glGenBuffers(1, &textureIndexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, textureIndexBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_texture_indices.size() * sizeof(unsigned int), &m_texture_indices[0], GL_STATIC_DRAW);
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 0, (void *)nullptr);
        // 4th attribute buffer : Texture Indices
        glEnableVertexAttribArray(3);
        // Polygon Flags
        glGenBuffers(1, &polyFlagBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, polyFlagBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_polygon_flags.size() * sizeof(uint32_t), &m_polygon_flags[0], GL_STATIC_DRAW);
        glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, 0, (void *)nullptr);
        // 5th attribute buffer : Polygon Flags
        glEnableVertexAttribArray(4);

        glBindVertexArray(0);

        return true;
    }
} // namespace OpenNFS
