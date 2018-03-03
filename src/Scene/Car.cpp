//
// Created by Amrik on 03/03/2018.
//

#include "Car.h"

Car::Car(std::string name, int model_id, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs,
         std::vector<glm::vec3> norms, std::vector<unsigned int> indices) : super(name, model_id, verts, uvs, norms, indices, true) {
        m_normals.clear();
        for (unsigned int m_vertex_index : m_vertex_indices) {
            m_normals.push_back(norms[m_vertex_index]);
        }
}

void Car::update() {
    position = glm::vec3(-31,0.07,-5);
    //position = glm::vec3(0, 0, 0);
    orientation_vec = glm::vec3(0,0,0);
    orientation = glm::normalize(glm::quat(orientation_vec));
    position = glm::vec3(position.x, position.y, position.z);
    RotationMatrix = glm::toMat4(orientation);
    TranslationMatrix = glm::translate(glm::mat4(1.0), position);
    ModelMatrix = TranslationMatrix * RotationMatrix;
}

void Car::destroy() {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
}

void Car::render() {
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
    // 3rd attribute buffer : Normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
            2,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_TRUE,           // normalized?
            0,                  // stride
            (void *) 0            // array buffer offset
    );
    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

bool Car::genBuffers() {
    // Verts
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
    // UVs
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);
    // Normals
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
    return true;
}






