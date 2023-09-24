#pragma once

#include "Geometry.h"

namespace LibOpenNFS {
    class TrackGeometry : public Geometry {
    public:
        TrackGeometry();
        TrackGeometry(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs, std::vector<uint32_t> &textureIndices,
                      std::vector<uint32_t> &vertexIndices, std::vector<glm::vec4> &shadingData, std::vector<uint32_t> &debugData, glm::vec3 centerPosition);
        TrackGeometry(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs, std::vector<uint32_t> &textureIndices,
                      std::vector<uint32_t> &vertexIndices, std::vector<glm::vec4> &shadingData, glm::vec3 centerPosition);

        std::vector<uint32_t> m_textureIndices;
        std::vector<glm::vec4> m_shadingData;
        std::vector<uint32_t> m_debugData;
    };
} // namespace LibOpenNFS
