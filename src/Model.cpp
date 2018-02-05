//
// Created by Amrik on 25/10/2017.
//


#include "Model.h"

Model::Model() = default;

Model::Model(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs,std::vector<glm::vec3> norms,std::vector<unsigned int> indices){
    m_name = std::move(name);
    m_vertices = std::move(verts);
    m_uvs = std::move(uvs);
    m_normals = std::move(norms);
    m_vertex_indices = std::move(indices);
}

Model::Model(std::string name):m_name(std::move(name)) {
};

std::vector<glm::vec2> Model::getUVs(void){
    return m_uvs;
}
std::vector<glm::vec3> Model::getVertices(void){
    return m_vertices;
}

std::vector<glm::vec3> Model::getNormals(void){
    return m_normals;
}


std::vector<unsigned int> Model::getIndices() {
    return m_vertex_indices;
}


std::string Model::getName(void){
    return m_name;
}

void Model::setUVs(std::vector<glm::vec2> uvs){
    m_uvs = std::move(uvs);
}

void Model::setVertices(std::vector<glm::vec3> verts){
    m_vertices = std::move(verts);
}

void Model::setNormals(std::vector<glm::vec3> norms){
    m_normals = std::move(norms);
}


void Model::setIndices(std::vector<unsigned int> indices){
    m_vertex_indices = std::move(indices);
}

void Model::update(){
    if (m_name.find("_") != std::string::npos){
        ModelMatrix = glm::rotate(ModelMatrix, 0.001f, glm::vec3(1, 0, 0));
    } else{
        ModelMatrix = glm::rotate(ModelMatrix, 0.001f, glm::vec3(0, 1, 0));
    }
}

void Model::enable(){
    enabled = true;
}

void Model::destroy(){
    glDeleteBuffers(1, &gl_buffers.vertexbuffer);
    glDeleteBuffers(1, &gl_buffers.uvbuffer);
    glDeleteBuffers(1, &gl_buffers.normalbuffer);
}

void Model::render(){
    if (!enabled)
        return;

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
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

bool Model::genBuffers(){
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

