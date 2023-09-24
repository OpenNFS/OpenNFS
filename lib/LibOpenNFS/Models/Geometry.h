#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <utility>
#include <vector>
#include <cstdlib>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace LibOpenNFS {
    class Geometry {
    public:
        Geometry(std::string name, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec2> &uvs, const std::vector<glm::vec3> &normals,
                 const std::vector<uint32_t> &vertexIndices, bool removeVertexIndexing, const glm::vec3 &centerPosition);
        std::string m_name;
        std::vector<glm::vec3> m_vertices;
        std::vector<glm::vec3> m_normals;
        std::vector<glm::vec2> m_uvs;
        std::vector<uint32_t> m_vertexIndices;

        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 RotationMatrix {};
        glm::mat4 TranslationMatrix {};
        glm::vec3 position {};
        glm::vec3 initialPosition {};
        glm::vec3 orientation_vec {};
        glm::quat orientation {};
    };
} // namespace LibOpenNFS
