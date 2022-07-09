#include "Model.h"

#include <utility>

Model::Model(std::string name,
             std::vector<glm::vec3>
               vertices,
             std::vector<glm::vec2>
               uvs,
             std::vector<glm::vec3>
               normals,
             std::vector<uint32_t>
               vertexIndices,
             bool removeVertexIndexing,
             glm::vec3 centerPosition) :
    m_name(std::move(name)), m_uvs(std::move(uvs)), m_vertexIndices(std::move(vertexIndices)), m_normals(std::move(normals)) {
    if (removeVertexIndexing) {
        for (unsigned int m_vertex_index : m_vertexIndices) {
            m_vertices.push_back(vertices[m_vertex_index]);
        }
    } else {
        m_vertices = std::move(vertices);
    }

    position        = centerPosition;
    initialPosition = centerPosition;
    orientation_vec = glm::vec3(0, 0, 0);
    orientation     = glm::normalize(glm::quat(orientation_vec));
}

void Model::enable() {
    enabled = true;
}
