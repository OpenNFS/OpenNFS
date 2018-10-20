//
// Created by Amrik Sadhra on 03/03/2018.
//

#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include "Track.h"
#include "../Util/Utils.h"

Track::Track(std::vector<glm::vec3> verts, std::vector<glm::vec3> norms, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices, std::vector<glm::vec4> shading_data, std::vector<uint32_t> debug_data, glm::vec3 center_position) : super("TrackMesh", verts, uvs, norms, indices, true, center_position) {
    m_texture_indices = texture_indices;
    shadingData = shading_data;
    m_debug_data = debug_data;
    // Index Shading data
    for(unsigned int m_vertex_index : indices) {
        m_shading_data.push_back(shading_data[m_vertex_index]);
    }
    enable();
    ASSERT(genBuffers(), "Unable to generate GL Buffers for Track Model");
}

Track::Track(std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices,
             std::vector<glm::vec4> shading_data, glm::vec3 center_position) : super("TrackMesh", verts, uvs, std::vector<glm::vec3>(), indices, true, center_position){
    m_texture_indices = texture_indices;
    shadingData = shading_data;

    // Fill the unused buffer with data
    for(int i = 0; i < m_texture_indices.size(); ++i){
        m_debug_data.emplace_back(0);
    }
    for(int i = 0; i < m_texture_indices.size(); ++i){
        m_normals.emplace_back(glm::vec3(1,1,1));
    }
    // Index Shading data
    for(unsigned int m_vertex_index : indices) {
        m_shading_data.push_back(shading_data[m_vertex_index]);
    }
    enable();
    ASSERT(genBuffers(), "Unable to generate GL Buffers for Track Model");
}

Track::Track(std::vector<glm::vec3> verts, std::vector<glm::vec3> norms, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices,
             std::vector<glm::vec4> shading_data, glm::vec3 center_position) : super("TrackMesh", verts, uvs, norms, indices, true, center_position){
    m_texture_indices = texture_indices;
    shadingData = shading_data;
    // Fill the unused buffer with data
    for(int i = 0; i < m_texture_indices.size(); ++i){
        m_debug_data.emplace_back(0);
    }
    // Index Shading data
    for(unsigned int m_vertex_index : indices) {
        m_shading_data.push_back(shading_data[m_vertex_index]);
    }
    enable();
    ASSERT(genBuffers(), "Unable to generate GL Buffers for Track Model");
}

void Track::update() {
    //orientation_vec = glm::vec3(0,0,0);
    //orientation = glm::normalize(glm::quat(orientation_vec));
    RotationMatrix = glm::toMat4(orientation);
    TranslationMatrix = glm::translate(glm::mat4(1.0), position);
    ModelMatrix = TranslationMatrix * RotationMatrix;
}


void Track::destroy() {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &textureIndexBuffer);
    glDeleteBuffers(1, &shadingBuffer);
    glDeleteBuffers(1, &normalBuffer);
    glDeleteBuffers(1, &debugBuffer);
}

void Track::render() {
    if (enabled){
        glBindVertexArray(VertexArrayID);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) m_vertices.size());
        glBindVertexArray(0);
    }
}

bool Track::genBuffers() {
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    // 1st attribute buffer : Vertices
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void *) 0            // array buffer offset
    );
    glEnableVertexAttribArray(0);
    // 2nd attribute buffer : UVs
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
            1,                                // attribute
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void *) 0                          // array buffer offset
    );
    glEnableVertexAttribArray(1);
    // 3rd attribute buffer : Track Normals
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
            2,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void *) 0            // array buffer offset
    );
    glEnableVertexAttribArray(2);
    // 4th attribute buffer : Texture Indices
    glGenBuffers(1, &textureIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureIndexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_texture_indices.size() * sizeof(unsigned int), &m_texture_indices[0], GL_STATIC_DRAW);
    glVertexAttribIPointer(
            3,
            1,
            GL_UNSIGNED_INT,
            0,
            (void *) 0
    );
    glEnableVertexAttribArray(3);
    // 5th attribute buffer : NFS Shading Data
    glGenBuffers(1, &shadingBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadingBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_shading_data.size() * sizeof(glm::vec4), &m_shading_data[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
            4,
            4,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) 0
    );
    glEnableVertexAttribArray(4);
    // 6th attribute buffer : Debug Data
    // Normals
    glGenBuffers(1, &debugBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, debugBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_debug_data.size() * sizeof(uint32_t), &m_debug_data[0], GL_STATIC_DRAW);
    glVertexAttribIPointer(
            5,
            1,
            GL_UNSIGNED_INT,
            0,
            (void *) 0
    );
    glEnableVertexAttribArray(5);
    // Lets not affect any state
    glBindVertexArray(0);
    return true;
}

Track::Track() : super("Track", std::vector<glm::vec3>(), std::vector<glm::vec2>(), std::vector<glm::vec3>(), std::vector<unsigned int>(), false, glm::vec3(0,0,0)){}
