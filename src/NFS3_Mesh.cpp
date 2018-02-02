//
// Created by Amrik on 25/10/2017.
//

#include "NFS3_Mesh.h"

#include <utility>
#include <GL/glew.h>

NFS3_Mesh::NFS3_Mesh() = default;

NFS3_Mesh::NFS3_Mesh(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs,std::vector<glm::vec3> norms,std::vector<unsigned int> indices){
    m_name = std::move(name);
    m_vertices = std::move(verts);
    m_uvs = std::move(uvs);
    m_normals = std::move(norms);
    m_vertex_indices = std::move(indices);
}

NFS3_Mesh::NFS3_Mesh(std::string name):m_name(std::move(name)) {
};

std::vector<glm::vec2> NFS3_Mesh::getUVs(void){
    return m_uvs;
}
std::vector<glm::vec3> NFS3_Mesh::getVertices(void){
    return m_vertices;
}

std::vector<glm::vec3> NFS3_Mesh::getNormals(void){
    return m_normals;
}


std::vector<unsigned int> NFS3_Mesh::getIndices() {
    return m_vertex_indices;
}


std::string NFS3_Mesh::getName(void){
    return m_name;
}

void NFS3_Mesh::setUVs(std::vector<glm::vec2> uvs){
    m_uvs = std::move(uvs);
}

void NFS3_Mesh::setVertices(std::vector<glm::vec3> verts){
    m_vertices = std::move(verts);
}

void NFS3_Mesh::setNormals(std::vector<glm::vec3> norms){
    m_normals = std::move(norms);
}


void NFS3_Mesh::setIndices(std::vector<unsigned int> indices){
    m_vertex_indices = std::move(indices);
}

void NFS3_Mesh::destroy(){
    glDeleteBuffers(1, &gl_buffers.vertexbuffer);
    glDeleteBuffers(1, &gl_buffers.uvbuffer);
    glDeleteBuffers(1, &gl_buffers.normalbuffer);
}

void NFS3_Mesh::render(){
    // 1st attribute buffer : car_verts
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, gl_buffers.vertexbuffer);
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
    glBindBuffer(GL_ARRAY_BUFFER, gl_buffers.uvbuffer);
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
    glBindBuffer(GL_ARRAY_BUFFER, gl_buffers.normalbuffer);
    glVertexAttribPointer(
            2,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_TRUE,           // normalized?
            0,                  // stride
            (void *) 0            // array buffer offset
    );
    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_buffers.elementbuffer);
    // Draw the triangles !
    glDrawElements(
            GL_TRIANGLES,      // mode
            m_vertex_indices.size(),    // count
            GL_UNSIGNED_INT,   // type
            (void *) 0           // element array buffer offset
    );
}

bool NFS3_Mesh::genBuffers(){
    // Load car_verts into a VBO
    glGenBuffers(1, &gl_buffers.vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gl_buffers.vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
    // UVs
    glGenBuffers(1, &gl_buffers.uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gl_buffers.uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);
    // Indices
    glGenBuffers(1, &gl_buffers.elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_buffers.elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vertex_indices.size() * sizeof(unsigned int), &m_vertex_indices[0], GL_STATIC_DRAW);
    // Normals
    glGenBuffers(1, &gl_buffers.normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gl_buffers.normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
    return true;
}

