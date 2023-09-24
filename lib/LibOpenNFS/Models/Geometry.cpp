#include "Geometry.h"

#include <utility>

namespace LibOpenNFS {
    Geometry::Geometry(std::string name,
                       const std::vector<glm::vec3> &vertices,
                       const std::vector<glm::vec2> &uvs,
                       const std::vector<glm::vec3> &normals,
                       const std::vector<uint32_t> &vertexIndices,
                       bool removeVertexIndexing,
                       const glm::vec3 &centerPosition) :
        m_name(std::move(name)), m_uvs(uvs), m_vertexIndices(vertexIndices), m_normals(normals) {
        if (removeVertexIndexing) {
            for (auto const &vertex_index : m_vertexIndices) {
                m_vertices.push_back(vertices[vertex_index]);
            }
        } else {
            m_vertices = vertices;
        }

        position        = centerPosition;
        initialPosition = centerPosition;
        orientation_vec = glm::vec3(0, 0, 0);
        orientation     = glm::normalize(glm::quat(orientation_vec));
    }
} // namespace LibOpenNFS
