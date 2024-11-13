#include "TrackGeometry.h"

namespace LibOpenNFS {
    TrackGeometry::TrackGeometry() :
        Geometry("TrackModel", std::vector<glm::vec3>(), std::vector<glm::vec2>(), std::vector<glm::vec3>(), std::vector<unsigned int>(), false, glm::vec3(0, 0, 0)) {
    }
    TrackGeometry::TrackGeometry(std::vector<glm::vec3> &vertices,
                                 std::vector<glm::vec3> &normals,
                                 std::vector<glm::vec2> &uvs,
                                 std::vector<uint32_t> &textureIndices,
                                 std::vector<uint32_t> &vertexIndices,
                                 std::vector<glm::vec4> &shadingData,
                                 std::vector<uint32_t> &debugData,
                                 glm::vec3 centerPosition) :
        m_textureIndices(textureIndices), m_debugData(debugData), Geometry("TrackMesh", vertices, uvs, normals, vertexIndices, true, centerPosition) {
        // Index Shading data
        for (uint32_t m_vertex_index : vertexIndices) {
            m_shadingData.push_back(shadingData[m_vertex_index]);
        }
    }

    TrackGeometry::TrackGeometry(std::vector<glm::vec3> &vertices,
                                 std::vector<glm::vec3> &normals,
                                 std::vector<glm::vec2> &uvs,
                                 std::vector<uint32_t> &textureIndices,
                                 std::vector<uint32_t> &vertexIndices,
                                 std::vector<glm::vec4> &shadingData,
                                 glm::vec3 centerPosition) :
        m_textureIndices(textureIndices), Geometry("TrackMesh", vertices, uvs, normals, vertexIndices, true, centerPosition) {
        // Fill the unused buffer with data
        m_debugData.resize(m_textureIndices.size());

        // Index Shading data
        for (const auto &vertexIndex : vertexIndices) {
            m_shadingData.push_back(shadingData[vertexIndex]);
        }
    }

} // namespace LibOpenNFS
