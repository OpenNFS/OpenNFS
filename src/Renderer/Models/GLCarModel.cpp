#include "GLCarModel.h"
#include "../../Util/Utils.h"

namespace OpenNFS {
    GLCarModel::GLCarModel(LibOpenNFS::CarGeometry *geometry, const float specular_damper, const float specular_reflectivity, const float env_reflectivity) : GLModel(geometry) {
        // Can't call basic constructor as genBuffers() call would run before m_textureIndices was available
        specularDamper       = specular_damper;
        specularReflectivity = specular_reflectivity;
        envReflectivity      = env_reflectivity;
    };

    GLCarModel::GLCarModel(LibOpenNFS::CarGeometry *geometry) :
        GLModel(geometry){

        };

    void GLCarModel::update() {
        // TODO: Move this to a separate function, as not all car models should be enabled (low LOD etc)
        if (!enabled) {
            enable();
            CHECK_F(genBuffers(), "Unable to generate GL Buffers for Track Model");
        }
        geometry->RotationMatrix    = glm::toMat4(geometry->orientation);
        geometry->TranslationMatrix = glm::translate(glm::mat4(1.0), geometry->position);
        geometry->ModelMatrix       = geometry->TranslationMatrix * geometry->RotationMatrix;
    }

    void GLCarModel::destroy() {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &uvBuffer);
        glDeleteBuffers(1, &normalBuffer);
        glDeleteBuffers(1, &textureIndexBuffer);
        glDeleteBuffers(1, &polyFlagBuffer);
    }

    void GLCarModel::render() {
        if (enabled) {
            glBindVertexArray(VertexArrayID);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei) geometry->m_vertices.size());
            glBindVertexArray(0);
        }
    }

    bool GLCarModel::genBuffers() {
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        // Verts
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, geometry->m_vertices.size() * sizeof(glm::vec3), &(geometry->m_vertices[0]), GL_STATIC_DRAW);
        glVertexAttribPointer(0,         // attribute
                              3,         // size
                              GL_FLOAT,  // type
                              GL_FALSE,  // normalized?
                              0,         // stride
                              (void *) 0 // array buffer offset
        );
        // 1st attribute buffer : Vertices
        glEnableVertexAttribArray(0);
        // UVs
        glGenBuffers(1, &uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, geometry->m_uvs.size() * sizeof(glm::vec2), &(geometry->m_uvs[0]), GL_STATIC_DRAW);
        glVertexAttribPointer(1,         // attribute
                              2,         // size
                              GL_FLOAT,  // type
                              GL_FALSE,  // normalized?
                              0,         // stride
                              (void *) 0 // array buffer offset
        );
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        // Normals
        glGenBuffers(1, &normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, geometry->m_normals.size() * sizeof(glm::vec3), &(geometry->m_normals[0]), GL_STATIC_DRAW);
        glVertexAttribPointer(2,         // attribute
                              3,         // size
                              GL_FLOAT,  // type
                              GL_TRUE,   // normalized?
                              0,         // stride
                              (void *) 0 // array buffer offset
        );
        // 3rd attribute buffer : Normals
        glEnableVertexAttribArray(2);
        // Texture Indices
        glGenBuffers(1, &textureIndexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, textureIndexBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<LibOpenNFS::CarGeometry *>(geometry)->m_texture_indices.size() * sizeof(unsigned int),
                     &(static_cast<LibOpenNFS::CarGeometry *>(geometry)->m_texture_indices[0]),
                     GL_STATIC_DRAW);
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 0, (void *) 0);
        // 4th attribute buffer : Texture Indices
        glEnableVertexAttribArray(3);
        // Polygon Flags
        glGenBuffers(1, &polyFlagBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, polyFlagBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<LibOpenNFS::CarGeometry *>(geometry)->m_polygon_flags.size() * sizeof(uint32_t),
                     &(static_cast<LibOpenNFS::CarGeometry *>(geometry)->m_polygon_flags[0]),
                     GL_STATIC_DRAW);
        glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, 0, (void *) 0);
        // 5th attribute buffer : Polygon Flags
        glEnableVertexAttribArray(4);

        glBindVertexArray(0);

        return true;
    }

} // namespace OpenNFS
