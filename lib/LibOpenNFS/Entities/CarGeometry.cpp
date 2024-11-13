#include "CarGeometry.h"

namespace LibOpenNFS {
    // TODO: These all kinda do the same thing, so kill the extras
    CarGeometry::CarGeometry(const std::string& name,
                             const std::vector<glm::vec3>& verts,
                             const std::vector<glm::vec2>& uvs,
                             const std::vector<uint32_t>& texture_indices,
                             const std::vector<uint32_t>& test,
                             const std::vector<glm::vec3>& norms,
                             const std::vector<uint32_t>& indices,
                             const glm::vec3& center_position) :
        Geometry(name, verts, uvs, norms, indices, true, center_position) {
        m_texture_indices = texture_indices;
        isMultiTextured   = true;
        // Fill the unused buffer with data
        m_polygon_flags = test;
        m_normals.clear();
        for (auto const& vert_index : m_vertexIndices) {
            m_normals.push_back(norms[vert_index]);
        }
    }

    CarGeometry::CarGeometry(const std::string& name,
                             const std::vector<glm::vec3>& verts,
                             const std::vector<glm::vec2>& uvs,
                             const std::vector<uint32_t>& texture_indices,
                             const std::vector<glm::vec3>& norms,
                             const std::vector<uint32_t>& indices,
                             const glm::vec3& center_position) :
        Geometry(name, verts, uvs, norms, indices, true, center_position) {
        m_texture_indices = texture_indices;
        isMultiTextured   = true;
        m_polygon_flags.resize(m_texture_indices.size());
        m_normals = norms;
    }

    CarGeometry::CarGeometry(const std::string& name,
                             const std::vector<glm::vec3>& verts,
                             const std::vector<glm::vec2>& uvs,
                             const std::vector<glm::vec3>& norms,
                             const std::vector<uint32_t>& indices,
                             const std::vector<uint32_t>& poly_flags,
                             const glm::vec3& center_position) :
        Geometry(name, verts, uvs, norms, indices, true, center_position) {
        m_polygon_flags = poly_flags;
        m_texture_indices.resize(m_vertexIndices.size());
        m_normals.clear();
        for (const auto& vertex_index : m_vertexIndices) {
            m_normals.push_back(norms[vertex_index]);
        }
    }

    CarGeometry::CarGeometry(const std::string& name,
                             const std::vector<glm::vec3>& verts,
                             const std::vector<glm::vec2>& uvs,
                             const std::vector<glm::vec3>& norms,
                             const std::vector<uint32_t>& indices,
                             const glm::vec3& center_position) :
        Geometry(name, verts, uvs, norms, indices, false, center_position) {
        m_texture_indices.resize(m_normals.size());
        m_polygon_flags.resize(m_texture_indices.size());
        m_normals.clear();
        for (const auto& vertex_index : m_vertexIndices) {
            m_normals.push_back(norms[vertex_index]);
        }
    }
} // namespace LibOpenNFS