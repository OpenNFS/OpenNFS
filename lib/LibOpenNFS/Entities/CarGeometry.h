#pragma once

#include "Geometry.h"

namespace LibOpenNFS {
    class CarGeometry : public Geometry {
    public:
        CarGeometry(const std::string& name,
                    const std::vector<glm::vec3>& verts,
                    const std::vector<glm::vec2>& uvs,
                    const std::vector<uint32_t>& texture_indices,
                    const std::vector<uint32_t>& test,
                    const std::vector<glm::vec3>& norms,
                    const std::vector<uint32_t>& indices,
                    const glm::vec3& center_position);
        // Multitextured Cars (NFS2)
        CarGeometry(const std::string& name,
                    const std::vector<glm::vec3>& verts,
                    const std::vector<glm::vec2>& uvs,
                    const std::vector<uint32_t>& texture_indices,
                    const std::vector<glm::vec3>& norms,
                    const std::vector<uint32_t>& indices,
                    const glm::vec3& center_position);
        // Cars with Per-Polygon Flags (NFS4)
        CarGeometry(const std::string& name,
                    const std::vector<glm::vec3>& verts,
                    const std::vector<glm::vec2>& uvs,
                    const std::vector<glm::vec3>& norms,
                    const std::vector<uint32_t>& indices,
                    const std::vector<uint32_t>& poly_flags,
                    const glm::vec3& center_position);
        // Vanilla Cars (NFS3)
        CarGeometry(const std::string& name,
                    const std::vector<glm::vec3>& verts,
                    const std::vector<glm::vec2>& uvs,
                    const std::vector<glm::vec3>& norms,
                    const std::vector<uint32_t>& indices,
                    const glm::vec3& center_position);

        // Multitextured Car
        bool isMultiTextured = false;
        std::vector<unsigned int> m_texture_indices;
        // NFS4 Car
        std::vector<uint32_t> m_polygon_flags;
    };
} // namespace LibOpenNFS