//
// Created by Amrik on 03/03/2018.
//


#include "CarModel.h"
#include "../Util/Utils.h"

CarModel::CarModel(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms, std::vector<unsigned int> indices, glm::vec3 center_position, float specular_damper, float specular_reflectivity, float env_reflectivity) : super(name, verts, uvs, norms, indices, true, center_position) {
    specularDamper = specular_damper;
    specularReflectivity = specular_reflectivity;
    envReflectivity = env_reflectivity;
    m_normals.clear();
    for (unsigned int m_vertex_index : m_vertex_indices) {
        m_normals.push_back(norms[m_vertex_index]);
    }

    // Gen VBOs, add to Bullet Physics
    ASSERT(genBuffers(), "Unable to generate GL Buffers for Car Model ");
}

void CarModel::update() {
    RotationMatrix = glm::toMat4(orientation);
    TranslationMatrix = glm::translate(glm::mat4(1.0), position);
    ModelMatrix = TranslationMatrix * RotationMatrix;
}

void CarModel::destroy() {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
}

void CarModel::render() {
    if (enabled){
        glBindVertexArray(VertexArrayID);
        glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
        glBindVertexArray(0);
    }
}

bool CarModel::genBuffers() {
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    // Verts
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
    // 1st attribute buffer : Vertices
    glEnableVertexAttribArray(0);
    // UVs
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
    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    // Normals
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
            2,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_TRUE,           // normalized?
            0,                  // stride
            (void *) 0            // array buffer offset
    );
    // 3rd attribute buffer : Normals
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    return true;
}

CarModel::CarModel() : super("CarModel", std::vector<glm::vec3>(), std::vector<glm::vec2>(), std::vector<glm::vec3>(), std::vector<unsigned int>(), false, glm::vec3(0,0,0)){}




