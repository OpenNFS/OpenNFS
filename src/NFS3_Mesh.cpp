//
// Created by Amrik on 25/10/2017.
//

#include "NFS3_Mesh.h"

#include <utility>
#include <GL/glew.h>

NFS3_Mesh::NFS3_Mesh() = default;

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

unsigned int NFS3_Mesh::getVAO(){

}

