#include "GLTrackModel.h"
#include "../../Util/Utils.h"

GLTrackModel::~GLTrackModel() {
    if (buffersGenerated) {
        glDeleteBuffers(1, &m_vertexBuffer);
        glDeleteBuffers(1, &m_uvBuffer);
        glDeleteBuffers(1, &m_textureIndexBuffer);
        glDeleteBuffers(1, &m_shadingDataBuffer);
        glDeleteBuffers(1, &m_normalBuffer);
        glDeleteBuffers(1, &m_debugBuffer);
    }
}

void GLTrackModel::UpdateMatrices() {
    if (!buffersGenerated) {
        buffersGenerated = true;
        CHECK_F(GenBuffers(), "Unable to generate GL Buffers for %s Model", m_name.c_str());
    }
    RotationMatrix = glm::toMat4(orientation);
    TranslationMatrix = glm::translate(glm::mat4(1.0), position);
    ModelMatrix = TranslationMatrix * RotationMatrix;
}

void GLTrackModel::Render() {
    if (enabled) {
        glBindVertexArray(VertexArrayID);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) m_vertices.size());
        glBindVertexArray(0);
    }
}

bool GLTrackModel::GenBuffers() {
    auto trackGeometry = dynamic_cast<LibOpenNFS::TrackGeometry *>(this);

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    // 1st attribute buffer : Vertices
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, // attribute
                          3, // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0, // stride
                          (void *) nullptr // array buffer offset
    );
    glEnableVertexAttribArray(0);
    // 2nd attribute buffer : UVs
    glGenBuffers(1, &m_uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, // attribute
                          2, // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0, // stride
                          (void *) nullptr // array buffer offset
    );
    glEnableVertexAttribArray(1);
    // 3rd attribute buffer : TrackModel Normals
    glGenBuffers(1, &m_normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, // attribute
                          3, // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0, // stride
                          (void *) nullptr // array buffer offset
    );
    glEnableVertexAttribArray(2);
    // 4th attribute buffer : Texture Indices
    glGenBuffers(1, &m_textureIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_textureIndexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 trackGeometry->m_textureIndices.size() * sizeof(uint32_t),
                 &trackGeometry->m_textureIndices[0],
                 GL_STATIC_DRAW);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 0, (void *) nullptr);
    glEnableVertexAttribArray(3);
    // 5th attribute buffer : NFS Shading Data
    glGenBuffers(1, &m_shadingDataBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_shadingDataBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 trackGeometry->m_shadingData.size() * sizeof(glm::vec4),
                 &trackGeometry->m_shadingData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, (void *) nullptr);
    glEnableVertexAttribArray(4);
    // 6th attribute buffer : Debug Data
    // Normals
    glGenBuffers(1, &m_debugBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_debugBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 trackGeometry->m_debugData.size() * sizeof(uint32_t),
                 &trackGeometry->m_debugData[0],
                 GL_STATIC_DRAW);
    glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, 0, (void *) nullptr);
    glEnableVertexAttribArray(5);
    // Lets not affect any state
    glBindVertexArray(0);
    return true;
}
