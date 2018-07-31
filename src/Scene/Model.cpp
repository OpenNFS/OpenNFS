//
// Created by Amrik on 25/10/2017.
//

#include "Model.h"

#include <utility>

Model::Model(std::string name, std::vector<glm::vec3> verts, std::vector<glm::vec2> uvs, std::vector<glm::vec3> norms, std::vector<unsigned int> indices, bool removeVertexIndexing, glm::vec3 center_position) {
    m_name = std::move(name);
    m_uvs = std::move(uvs);
    m_vertex_indices = std::move(indices);
    m_normals = std::move(norms);

    if (removeVertexIndexing) {
        for (unsigned int m_vertex_index : m_vertex_indices) {
            m_vertices.push_back(verts[m_vertex_index]);
        }
    } else {
        m_vertices = std::move(verts);
    }

    position = center_position;
    orientation_vec = glm::vec3(0,0,0);
    orientation = glm::normalize(glm::quat(orientation_vec));
}

void Model::enable() {
    enabled = true;
}
