//
// Created by Amrik on 03/03/2018.
//

#include "Track.h"

Track::Track(std::string name, int model_id, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices, std::vector<short> tex_ids,
             std::vector<glm::vec4> shading_data, glm::vec3 center_position) : super(name, model_id, verts, uvs, std::vector<glm::vec3>(), indices, true, center_position){
    m_texture_indices = texture_indices;
    shadingData = shading_data;
    texture_ids = tex_ids;
    // Index Shading data
    for(unsigned int m_vertex_index : indices) {
        m_shading_data.push_back(shading_data[m_vertex_index]);
    }
}

Track::Track(std::string name, int model_id, std::vector<glm::vec3> verts, std::vector<glm::vec3> norms, std::vector<glm::vec2> uvs, std::vector<unsigned int> texture_indices, std::vector<unsigned int> indices, std::vector<short> tex_ids,
             std::vector<glm::vec4> shading_data, glm::vec3 center_position) : super(name, model_id, verts, uvs, norms, indices, true, center_position){
    m_texture_indices = texture_indices;
    shadingData = shading_data;
    texture_ids = tex_ids;
    m_normals.clear();
    // Index Shading data
    for(unsigned int m_vertex_index : indices) {
        m_shading_data.push_back(shading_data[m_vertex_index]);
        m_normals.push_back(norms[m_vertex_index]);
    }
}

void Track::update() {
    orientation_vec = glm::vec3(-SIMD_PI/2,0,0);
    orientation = glm::normalize(glm::quat(orientation_vec));
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
}

void Track::render() {
    if (!enabled)
        return;

    // 1st attribute buffer : Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void *) 0            // array buffer offset
    );
    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
            1,                                // attribute
            2,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void *) 0                          // array buffer offset
    );
    // 3rd attribute buffer : Texture Indices
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, textureIndexBuffer);
    glVertexAttribIPointer(
            2,
            1,
            GL_UNSIGNED_INT,
            0,
            (void *) 0
    );
    // 4th attribute buffer : NFS3 Shading Data
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, shadingBuffer);
    glVertexAttribPointer(
            3,
            4,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) 0
    );
    // 5th attribute buffer : Track Normals
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glVertexAttribPointer(
            4,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void *) 0            // array buffer offset
    );
    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
}

bool Track::genBuffers() {
    // Verts
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
    // UVs
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);
    // Texture Indices
    glGenBuffers(1, &textureIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureIndexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_texture_indices.size() * sizeof(unsigned int), &m_texture_indices[0], GL_STATIC_DRAW);
    // NFS3 Shading Data
    glGenBuffers(1, &shadingBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadingBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_shading_data.size() * sizeof(glm::vec4), &m_shading_data[0], GL_STATIC_DRAW);
    // Normals
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
    return true;
}

